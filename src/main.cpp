#include "animator.hpp"
#include "buttons.hpp"
#include "holly_jolly_cfg.hpp"
#include "pico/multicore.h"
#include "pico/stdio_usb.h"
#include "pico/stdlib.h"
#include "pico_debug.h"
#include "ws2812.hpp"
#include <cstring>


static void core0_entry()
{
  /*---------------------------------------------------------------------------
  Initialize hardware resources and the animator subsystem
  ---------------------------------------------------------------------------*/
  LED::initialize();
  Buttons::initialize();
  Animator::initialize();

  while( 1 )
  {
    sleep_ms( FRAME_REFRESH_RATE_MS );
    Buttons::process();
    Animator::process();
  }
}


static void core1_entry()
{
  /*---------------------------------------------------------------------------
  Initialize the USB port
  ---------------------------------------------------------------------------*/
  pico_debug_init();

  /*---------------------------------------------------------------------------
  Start the main loop
  ---------------------------------------------------------------------------*/
  pico_debug_core_x_thread();
}


int main()
{
  /*---------------------------------------------------------------------------
  Initialize system resources
  ---------------------------------------------------------------------------*/
  timer_hw->dbgpause = 0;    // Do not pause the timer during debug
  pico_debug_configure_clocks();

  /*---------------------------------------------------------------------------
  Start the secondary core
  ---------------------------------------------------------------------------*/
  multicore_launch_core1( core1_entry );

  /*---------------------------------------------------------------------------
  Main loop
  ---------------------------------------------------------------------------*/
  sleep_ms( 1000 );
  core0_entry();

  return 0;
}
