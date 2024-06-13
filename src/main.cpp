#include "animator.hpp"
#include "buttons.hpp"
#include "holly_jolly_cfg.hpp"
#include "pico/stdlib.h"
#include "pico/stdio_usb.h"
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
  Initialize hardware resources and the animator subsystem
  ---------------------------------------------------------------------------*/
  LED::initialize();
  Buttons::initialize();
  Animator::initialize();

  while( 1 )
  {
    sleep_ms( FRAME_REFRESH_RATE_MS );
    Animator::process();
  }
}