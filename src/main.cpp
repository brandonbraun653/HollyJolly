#include <cstring>

#include "pico/stdlib.h"
#include "hardware/dma.h"
#include "hardware/clocks.h"
#include "hardware/gpio.h"
#include "hardware/irq.h"
#include "pico/multicore.h"
#include "pico/sem.h"
#include "pico/mutex.h"

#include "ws2812.pio.h"


static constexpr uint LED_DATA_PIN    = 0;
static constexpr uint FREQ_800KHZ     = 800'000;
static constexpr uint PIO_SM          = 0;
static constexpr uint WS2812_NUM_LEDS = 15;
static PIO            PIO_INSTANCE    = pio0;
static int            dma_channel     = 0;

static uint32_t  raw_led_buffer[ 2 ][ WS2812_NUM_LEDS ];
static uint32_t *p_render_buffer  = nullptr;
static uint32_t *p_display_buffer = nullptr;


/**
 * @brief Initializes the PIO (Programmable I/O) for controlling WS2812 LEDs.
 */
static void init_pio()
{
  // Load the program into the pio peripheral
  uint pio_pgm_offset = pio_add_program( PIO_INSTANCE, &ws2812_program );

  // Initialize the GPIO pin that will be used to drive the WS2812 LEDs
  pio_gpio_init( PIO_INSTANCE, LED_DATA_PIN );
  pio_sm_set_consecutive_pindirs( PIO_INSTANCE, PIO_SM, LED_DATA_PIN, 1, true );

  pio_sm_config cfg = ws2812_program_get_default_config( pio_pgm_offset );

  // Set base pin to act on when the sideset command is executed
  sm_config_set_sideset_pins( &cfg, LED_DATA_PIN );

  // Set the OSR to shift 24 bits out before pulling more data from the TX FIFO.
  // Data format expected by the WS2812 LEDs is GRB with MSB first, so we need
  // to left shift to match properly with our system memory order (LSB).
  sm_config_set_out_shift( &cfg, false, true, 24 );

  // Set the FIFO join to TX so that the TX FIFO is used
  sm_config_set_fifo_join( &cfg, PIO_FIFO_JOIN_TX );

  // Configure the PIO clock so that programmed cycle times are correct
  int   cycles_per_bit = ws2812_T1 + ws2812_T2 + ws2812_T3;
  float div            = clock_get_hz( clk_sys ) / ( FREQ_800KHZ * cycles_per_bit );
  sm_config_set_clkdiv( &cfg, div );

  // Initialize and enable the PIO state machine
  pio_sm_init( PIO_INSTANCE, PIO_SM, pio_pgm_offset, &cfg );
  pio_sm_set_enabled( PIO_INSTANCE, PIO_SM, true );
}


/**
 * @brief Acknowledge the DMA transfer complete event
 */
static void dma_complete_callback()
{
  // This IRQ index selection is coupled with the init_dma() function
  dma_channel_acknowledge_irq0( dma_channel );
}


/**
 * @brief Initializes the DMA channel for transferring data to the PIO.
 *
 * The DMA channel is configured to read from the display buffer and write to
 * the PIO TX FIFO. The byte ordering is set up so that when combined with the
 * PIO configuration, data is piped out to the WS2812 LEDs in the correct order.
 *
 * Each transfer is a single burst and must be restarted manually to send a new
 * frame to the LEDs.
 */
static void init_dma()
{
  dma_channel_config dma_cfg = dma_channel_get_default_config( dma_channel );

  /*---------------------------------------------------------------------------
  Configure the DMA channel with the following settings:
  ---------------------------------------------------------------------------*/
  dma_channel = dma_claim_unused_channel( true );

  // Enable byte swapping to convert 32-bit data to MSB first
  channel_config_set_bswap( &dma_cfg, true );

  // Map the PIO tx fifo data request signal to the DMA channel
  channel_config_set_dreq( &dma_cfg, pio_get_dreq( PIO_INSTANCE, PIO_SM, true ) );

  // Set the transfer data size to 32 bits
  channel_config_set_transfer_data_size( &dma_cfg, DMA_SIZE_32 );

  // Set the read increment to increment the read address by 4 bytes
  channel_config_set_read_increment( &dma_cfg, true );

  // Set the write increment to not increment the write address
  channel_config_set_write_increment( &dma_cfg, false );

  // Set the transfer size to a single frame of led data
  dma_channel_set_trans_count( dma_channel, WS2812_NUM_LEDS, false );

  // Assign the DMA channel to read from the display buffer
  dma_channel_set_read_addr( dma_channel, p_display_buffer, false );

  // Assign the DMA channel to write to the PIO TX FIFO
  dma_channel_configure( dma_channel, &dma_cfg, &PIO_INSTANCE->txf[ PIO_SM ], nullptr, 0, false );
  dma_channel_start( dma_channel );

  /*---------------------------------------------------------------------------
  Handle the transfer complete event
  ---------------------------------------------------------------------------*/
  dma_channel_set_irq0_enabled( dma_channel, true );
  irq_set_exclusive_handler( DMA_IRQ_0, dma_complete_callback );
  irq_set_enabled( DMA_IRQ_0, true );
}


static void update_leds()
{
  /*---------------------------------------------------------------------------
  Render a new frame of LED data
  ---------------------------------------------------------------------------*/
  static uint32_t led_idx = 0;
  static uint32_t color = 0;

  memset( p_render_buffer, 0, sizeof( uint32_t ) * WS2812_NUM_LEDS );

  switch( color )
  {
    case 0:
      p_render_buffer[ led_idx ] = 0x110000; // b
      color++;
      break;
    case 1:
      p_render_buffer[ led_idx ] = 0x001100; // r
      color++;
      break;
    case 2:
      p_render_buffer[ led_idx ] = 0x00000A; // g
      color = 0;
      break;
  }

  led_idx = ( led_idx + 1 ) % WS2812_NUM_LEDS;

  /*---------------------------------------------------------------------------
  Wait for the current frame to finish before swapping buffers and kicking off
  a transfer of the next frame.
  ---------------------------------------------------------------------------*/
  dma_channel_wait_for_finish_blocking( dma_channel );

  uint32_t *p_temp = p_render_buffer;
  p_render_buffer  = p_display_buffer;
  p_display_buffer = p_temp;

  dma_channel_set_trans_count( dma_channel, WS2812_NUM_LEDS, false );
  dma_channel_set_read_addr( dma_channel, p_display_buffer, true );
}


int main()
{
  /*---------------------------------------------------------------------------
  Initialize system resources
  ---------------------------------------------------------------------------*/
  timer_hw->dbgpause = 0;    // Do not pause the timer during debug
  stdio_init_all();

  /*---------------------------------------------------------------------------
  Initialize module memory
  ---------------------------------------------------------------------------*/
  p_render_buffer  = &raw_led_buffer[ 0 ][ 0 ];
  p_display_buffer = &raw_led_buffer[ 1 ][ 0 ];
  memset( p_render_buffer, 0, sizeof( uint32_t ) * WS2812_NUM_LEDS );
  memset( p_display_buffer, 0, sizeof( uint32_t ) * WS2812_NUM_LEDS );

  /*---------------------------------------------------------------------------
  Configure the default LED pin
  ---------------------------------------------------------------------------*/
  gpio_init( PICO_DEFAULT_LED_PIN );
  gpio_set_dir( PICO_DEFAULT_LED_PIN, GPIO_OUT );

  /*---------------------------------------------------------------------------
  Configure the PIO to drive the WS2812 LEDs
  ---------------------------------------------------------------------------*/
  init_dma();
  init_pio();

  bool led_state = false;
  while( 1 )
  {
    update_leds();
    gpio_put( PICO_DEFAULT_LED_PIN, led_state = !led_state );
    sleep_ms( 75 );
  }
}