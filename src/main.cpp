#include "pico/stdlib.h"
#include "ws2812.hpp"
#include <cstring>

int main()
{
  /*---------------------------------------------------------------------------
  Initialize system resources
  ---------------------------------------------------------------------------*/
  timer_hw->dbgpause = 0;    // Do not pause the timer during debug
  stdio_init_all();

  /*---------------------------------------------------------------------------
  Configure the PIO to drive the WS2812 LEDs
  ---------------------------------------------------------------------------*/
  LED::initialize();

  uint32_t led_idx = 0;
  uint32_t color = 0;
  uint32_t *p_render_buffer = nullptr;

  while( 1 )
  {
    p_render_buffer = LED::getRenderBuffer();
    memset( p_render_buffer, 0, sizeof( uint32_t ) * LED::count() );

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

    led_idx = ( led_idx + 1 ) % LED::count();
    LED::swapBuffers();
    sleep_ms( 75 );
  }
}