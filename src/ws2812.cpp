/******************************************************************************
 *  File Name:
 *    ws2812.cpp
 *
 *  Description:
 *    Driver implementation for the string of LEDs on the board.
 *
 *  2024 | Brandon Braun | brandonbraun653@protonmail.com
 *****************************************************************************/

/*-----------------------------------------------------------------------------
Includes
-----------------------------------------------------------------------------*/
#include "hardware/clocks.h"
#include "hardware/dma.h"
#include "hardware/gpio.h"
#include "hardware/irq.h"
#include "pico/stdlib.h"
#include "ws2812.hpp"
#include "ws2812.pio.h"
#include <cstring>

/*---------------------------------------------------------------------------
Literals
---------------------------------------------------------------------------*/

#define PIO_INSTANCE ( pio0 )   // PIO instance to use for driving the LEDs

namespace LED
{
  /*---------------------------------------------------------------------------
  Constants
  ---------------------------------------------------------------------------*/

  static constexpr uint FREQ_800KHZ     = 800'000;    // 800kHz data rate
  static constexpr uint PIO_SM          = 0;          // PIO state machine index
  static constexpr uint WS2812_NUM_LEDS = 32;         // Number of LEDs in the string
  static constexpr uint WS2812_DATA_PIN = 23;         // GPIO pin to drive the LEDs

  /*---------------------------------------------------------------------------
  Variables
  ---------------------------------------------------------------------------*/

  static uint32_t  s_raw_led_buffer[ 2 ][ WS2812_NUM_LEDS ];    // Double buffered LED data
  static uint32_t *sp_render_buffer;                            // Pointer to the current render buffer
  static uint32_t *sp_display_buffer;                           // Pointer to the current display buffer
  static int       s_dma_channel;                               // DMA channel for transferring data to the PIO

  /*---------------------------------------------------------------------------
  Static Function Declarations
  ---------------------------------------------------------------------------*/

  static void dma_complete_callback();
  static void init_pio();
  static void init_dma();

  /*---------------------------------------------------------------------------
  Public Functions
  ---------------------------------------------------------------------------*/

  void initialize()
  {
    /*-------------------------------------------------------------------------
    Initialize the LED buffers and set the initial buffer pointers
    -------------------------------------------------------------------------*/
    s_dma_channel     = 0;
    sp_render_buffer  = &s_raw_led_buffer[ 0 ][ 0 ];
    sp_display_buffer = &s_raw_led_buffer[ 1 ][ 0 ];
    memset( sp_render_buffer, 0, sizeof( uint32_t ) * WS2812_NUM_LEDS );
    memset( sp_display_buffer, 0, sizeof( uint32_t ) * WS2812_NUM_LEDS );

    /*-------------------------------------------------------------------------
    Initialize the PIO and DMA peripherals
    -------------------------------------------------------------------------*/
    init_dma();
    init_pio();

    /*-------------------------------------------------------------------------
    Start the first frame transfer by clearing the display buffer
    -------------------------------------------------------------------------*/
    swapBuffers();
  }


  uint32_t count()
  {
    return WS2812_NUM_LEDS;
  }


  uint32_t *getRenderBuffer()
  {
    return sp_render_buffer;
  }


  void swapBuffers()
  {
    /*---------------------------------------------------------------------------
    Wait for the current frame to finish before swapping buffers and kicking off
    a transfer of the next frame.
    ---------------------------------------------------------------------------*/
    dma_channel_wait_for_finish_blocking( s_dma_channel );

    uint32_t *p_temp  = sp_render_buffer;
    sp_render_buffer  = sp_display_buffer;
    sp_display_buffer = p_temp;

    dma_channel_set_trans_count( s_dma_channel, WS2812_NUM_LEDS, false );
    dma_channel_set_read_addr( s_dma_channel, sp_display_buffer, true );
  }


  void resetBuffers()
  {
    dma_channel_wait_for_finish_blocking( s_dma_channel );

    memset( sp_render_buffer, 0, sizeof( uint32_t ) * WS2812_NUM_LEDS );
    memset( sp_display_buffer, 0, sizeof( uint32_t ) * WS2812_NUM_LEDS );
  }

  /*---------------------------------------------------------------------------
  Static Functions
  ---------------------------------------------------------------------------*/

  /**
   * @brief Initializes the PIO (Programmable I/O) for controlling WS2812 LEDs.
   */
  static void init_pio()
  {
    // Load the program into the pio peripheral
    uint pio_pgm_offset = pio_add_program( PIO_INSTANCE, &ws2812_program );

    // Initialize the GPIO pin that will be used to drive the WS2812 LEDs
    pio_gpio_init( PIO_INSTANCE, WS2812_DATA_PIN );
    pio_sm_set_consecutive_pindirs( PIO_INSTANCE, PIO_SM, WS2812_DATA_PIN, 1, true );

    pio_sm_config cfg = ws2812_program_get_default_config( pio_pgm_offset );

    // Set base pin to act on when the sideset command is executed
    sm_config_set_sideset_pins( &cfg, WS2812_DATA_PIN );

    // Set the OSR to shift 24 bits out before pulling more data from the TX FIFO.
    // Data format expected by the WS2812 LEDs is GRB with MSB first, so we need
    // to left shift to match properly with our system memory order (LSB).
    sm_config_set_out_shift( &cfg, false, true, 24 );

    // Set the FIFO join to TX so that the TX FIFO is used
    sm_config_set_fifo_join( &cfg, PIO_FIFO_JOIN_TX );

    // Configure the PIO clock so that programmed cycle times are correct. These
    // literals are coming from the PIO program generated by the pioasm tool.
    int   cycles_per_bit = ws2812_T1 + ws2812_T2 + ws2812_T3;
    float div            = clock_get_hz( clk_sys ) / ( FREQ_800KHZ * cycles_per_bit );
    sm_config_set_clkdiv( &cfg, div );

    // Initialize and enable the PIO state machine
    pio_sm_init( PIO_INSTANCE, PIO_SM, pio_pgm_offset, &cfg );
    pio_sm_set_enabled( PIO_INSTANCE, PIO_SM, true );
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
    /*---------------------------------------------------------------------------
    Configure the DMA channel with the following settings:
    ---------------------------------------------------------------------------*/
    s_dma_channel = dma_claim_unused_channel( true );
    auto dma_cfg  = dma_channel_get_default_config( s_dma_channel );

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
    dma_channel_set_trans_count( s_dma_channel, WS2812_NUM_LEDS, false );

    // Assign the DMA channel to read from the display buffer
    dma_channel_set_read_addr( s_dma_channel, sp_display_buffer, false );

    // Assign the DMA channel to write to the PIO TX FIFO
    dma_channel_configure( s_dma_channel, &dma_cfg, &PIO_INSTANCE->txf[ PIO_SM ], nullptr, 0, false );
    dma_channel_start( s_dma_channel );

    /*---------------------------------------------------------------------------
    Handle the transfer complete event
    ---------------------------------------------------------------------------*/
    dma_channel_set_irq0_enabled( s_dma_channel, true );
    irq_set_exclusive_handler( DMA_IRQ_0, dma_complete_callback );
    irq_set_enabled( DMA_IRQ_0, true );
  }


  /**
   * @brief Acknowledge the DMA transfer complete event
   */
  static void dma_complete_callback()
  {
    dma_channel_acknowledge_irq0( s_dma_channel );
  }
}    // namespace LED
