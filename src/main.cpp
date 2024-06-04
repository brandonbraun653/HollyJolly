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
static constexpr uint FREQ_400KHZ     = 800'000;
static constexpr uint PIO_SM          = 0;
static constexpr uint WS2812_NUM_LEDS = 15;
static PIO            PIO_INSTANCE    = pio0;
static int            dma_channel     = 0;

static uint32_t  raw_led_buffer[ 2 ][ WS2812_NUM_LEDS ];
static uint32_t *p_render_buffer  = nullptr;
static uint32_t *p_display_buffer = nullptr;

static critical_section_t s_cs;
static volatile bool s_swap_request;
static dma_channel_config dma_cfg;

/**
 * @brief Initialize the PIO to drive the WS2812 LEDs
 *
 */
static void init_pio()
{
  uint pio_pgm_offset = pio_add_program( PIO_INSTANCE, &ws2812_program );
  pio_gpio_init( PIO_INSTANCE, LED_DATA_PIN );
  pio_sm_set_consecutive_pindirs( PIO_INSTANCE, PIO_SM, LED_DATA_PIN, 1, true );

  pio_sm_config cfg = ws2812_program_get_default_config( pio_pgm_offset );

  // Set base pin to act on when the sideset command is executed
  sm_config_set_sideset_pins( &cfg, LED_DATA_PIN );

  // Set the OSR to shift 24 bits out before pulling more data from the TX FIFO.
  // Data format expected by the WS2812 LEDs is GRB with MSB first, so we need
  // to left shift to match properly.
  sm_config_set_out_shift( &cfg, false, true, 24 );

  // Set the FIFO join to TX so that the TX FIFO is used
  sm_config_set_fifo_join( &cfg, PIO_FIFO_JOIN_TX );

  // Configure the PIO clock so that programmed cycle times are correct
  int   cycles_per_bit = ws2812_T1 + ws2812_T2 + ws2812_T3;
  float div            = clock_get_hz( clk_sys ) / ( FREQ_400KHZ * cycles_per_bit );
  sm_config_set_clkdiv( &cfg, div );

  // Initialize and enable the PIO state machine
  pio_sm_init( PIO_INSTANCE, PIO_SM, pio_pgm_offset, &cfg );
  pio_sm_set_enabled( PIO_INSTANCE, PIO_SM, true );
}


/**
 * @brief Acknowledge the DMA transfer complete event
 *
 */
static void dma_complete_callback()
{
  dma_hw->ints0 = 1u << dma_channel;

  dma_channel_wait_for_finish_blocking( dma_channel );

  /*---------------------------------------------------------------------------
  Do the buffer swap
  ---------------------------------------------------------------------------*/
  if( s_swap_request )
  {
    s_swap_request = false;

    uint32_t *p_temp = p_render_buffer;
    p_render_buffer  = p_display_buffer;
    p_display_buffer = p_temp;
  }

  /*---------------------------------------------------------------------------
  Set up the parameters for the new transfer
  ---------------------------------------------------------------------------*/
  dma_channel_set_trans_count( dma_channel, WS2812_NUM_LEDS, false );
  dma_channel_set_read_addr( dma_channel, p_display_buffer, true );
  //dma_channel_start( dma_channel );
}


/**
 * @brief
 *
 */
static void init_dma()
{
  /*---------------------------------------------------------------------------
  Claim a DMA channel to use
  ---------------------------------------------------------------------------*/
  dma_channel = dma_claim_unused_channel( true );
  dma_cfg     = dma_channel_get_default_config( dma_channel );

  /*---------------------------------------------------------------------------
  Configure the DMA channel with the following settings:
  ---------------------------------------------------------------------------*/
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

  // Set the transfer size to 0 so that the DMA channel will loop indefinitely
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

  /*---------------------------------------------------------------------------
  Initialize data used for communicating between ISR and buffer swap requests
  ---------------------------------------------------------------------------*/
  critical_section_init( &s_cs );
  s_swap_request = false;
}


/**
 * @brief Performs a blocking swap of the render and display buffers.
 *
 * This will block until the next DMA transfer is complete, upon which the
 * callback ISR will swap the buffers and notify the rendering thread that
 * the new buffer is ready.
 */
static void buffer_swap()
{
  /*---------------------------------------------------------------------------
  Post a request to the DMA ISR to swap the buffers on the next frame transfer
  ---------------------------------------------------------------------------*/
  //critical_section_enter_blocking( &s_cs );
  s_swap_request = true;
  //critical_section_exit( &s_cs );

  /*---------------------------------------------------------------------------
  Just busy poll while waiting. This won't take long.
  ---------------------------------------------------------------------------*/
  while( s_swap_request == true )
  {
    sleep_ms( 25 );
  }
}


static void update_leds()
{
  static uint32_t led_idx = 0;

  memset( p_render_buffer, 0, sizeof( uint32_t ) * WS2812_NUM_LEDS );

  p_render_buffer[ led_idx ] = 0x00FF00;
  led_idx = ( led_idx + 1 ) % WS2812_NUM_LEDS;

  buffer_swap();
}


int main()
{
  /*---------------------------------------------------------------------------
  Initialize system resources
  ---------------------------------------------------------------------------*/
  timer_hw->dbgpause = 0; // Do not pause the timer during debug
  stdio_init_all();

  /*---------------------------------------------------------------------------
  Initialize module memory
  ---------------------------------------------------------------------------*/
  memset( &raw_led_buffer[ 0 ], 0, sizeof( uint32_t ) * WS2812_NUM_LEDS );
  memset( &raw_led_buffer[ 1 ], 0, sizeof( uint32_t ) * WS2812_NUM_LEDS );

  p_render_buffer  = raw_led_buffer[ 0 ];
  p_display_buffer = raw_led_buffer[ 1 ];
  s_swap_request   = false;

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

  // Allow some time for the leds to clear
  sleep_ms( 100 );

  // It seems like this is need to kick off the first transfer? Not sure why.
  pio_sm_put_blocking( PIO_INSTANCE, PIO_SM, 0 );

  bool led_state = false;
  while( 1 )
  {
    update_leds();
    gpio_put( PICO_DEFAULT_LED_PIN, led_state = !led_state );
    sleep_ms( 500 );
  }
}