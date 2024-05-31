/**
 * Copyright (c) 2020 Raspberry Pi (Trading) Ltd.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "pico/stdlib.h"
#include "hardware/clocks.h"
#include "hardware/gpio.h"
#include "pico/multicore.h"

#include "ws2812.pio.h"


static constexpr uint LED_DATA_PIN = 0;
static constexpr uint FREQ_400KHZ  = 800'000;
static constexpr uint PIO_SM       = 0;
static PIO            PIO_INSTANCE = pio0;


int main()
{
  timer_hw->dbgpause = 0;

  stdio_init_all();

  /*---------------------------------------------------------------------------
  Configure the default LED pin
  ---------------------------------------------------------------------------*/
  gpio_init( PICO_DEFAULT_LED_PIN );
  gpio_set_dir( PICO_DEFAULT_LED_PIN, GPIO_OUT );

  /*---------------------------------------------------------------------------
  Configure the PIO to drive the WS2812 LEDs
  ---------------------------------------------------------------------------*/
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
  int cycles_per_bit = ws2812_T1 + ws2812_T2 + ws2812_T3;
  float div          = clock_get_hz( clk_sys ) / ( FREQ_400KHZ * cycles_per_bit );
  sm_config_set_clkdiv( &cfg, div );

  // Initialize and enable the PIO state machine
  pio_sm_init( PIO_INSTANCE, PIO_SM, pio_pgm_offset, &cfg );
  pio_sm_set_enabled( PIO_INSTANCE, PIO_SM, true );
  sleep_ms( 5 );

  bool led_state = false;
  while( 1 )
  {
    // Light up 3 leds in red, green, and blue
    pio_sm_put_blocking( PIO_INSTANCE, PIO_SM, 0x00110000 );
    pio_sm_put_blocking( PIO_INSTANCE, PIO_SM, 0x11000000 );
    pio_sm_put_blocking( PIO_INSTANCE, PIO_SM, 0x00001100 );
    gpio_put( PICO_DEFAULT_LED_PIN, led_state = !led_state );
    sleep_ms( 10 );
  }
}