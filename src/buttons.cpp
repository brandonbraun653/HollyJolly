/******************************************************************************
 *  File Name:
 *    buttons.cpp
 *
 *  Description:
 *    Driver implementation for the physical buttons on the Holly Jolly
 *
 *  2024 | Brandon Braun | brandonbraun653@protonmail.com
 *****************************************************************************/

/*-----------------------------------------------------------------------------
Includes
-----------------------------------------------------------------------------*/
#include "buttons.hpp"
#include "hardware/gpio.h"
#include "pico/types.h"
#include <cstdint>

namespace Buttons
{
  /*---------------------------------------------------------------------------
  Constants
  ---------------------------------------------------------------------------*/

  static constexpr uint s_pin_input_bright = 19;
  static constexpr uint s_pin_input_action = 25;

  /*---------------------------------------------------------------------------
  Static Variables
  ---------------------------------------------------------------------------*/

  static volatile ButtonCallback brightPressCallback;
  static volatile ButtonCallback actionPressCallback;

  /*---------------------------------------------------------------------------
  Static Function Definitions
  ---------------------------------------------------------------------------*/

  /**
   * @brief IRQ handler to process button press events
   *
   * @param gpio  The GPIO pin that triggered the event
   * @param event_mask  The type of event that occurred
   */
  static void irqh_button_press( uint gpio, uint32_t event_mask )
  {
    const bool falling_edge = ( event_mask & GPIO_IRQ_EDGE_FALL ) == GPIO_IRQ_EDGE_FALL;

    if( ( gpio == s_pin_input_bright ) && falling_edge && brightPressCallback )
    {
      brightPressCallback();
    }
    else if( ( gpio == s_pin_input_action ) && falling_edge && actionPressCallback )
    {
      actionPressCallback();
    }
  }

  /*---------------------------------------------------------------------------
  Public Functions
  ---------------------------------------------------------------------------*/

  void initialize()
  {
    /*-------------------------------------------------------------------------
    Initialize the GPIO pins for button inputs
    -------------------------------------------------------------------------*/
    gpio_init( s_pin_input_bright );
    gpio_set_dir( s_pin_input_bright, GPIO_IN );
    gpio_pull_up( s_pin_input_bright );
    gpio_set_irq_enabled_with_callback( s_pin_input_bright, GPIO_IRQ_EDGE_FALL, true, irqh_button_press );

    gpio_init( s_pin_input_action );
    gpio_set_dir( s_pin_input_action, GPIO_IN );
    gpio_pull_up( s_pin_input_action );
    gpio_set_irq_enabled_with_callback( s_pin_input_action, GPIO_IRQ_EDGE_FALL, true, irqh_button_press );
  }

  void onBrightKeyPress( ButtonCallback callback )
  {
    brightPressCallback = callback;
  }

  void onActionKeyPress( ButtonCallback callback )
  {
    actionPressCallback = callback;
  }

}    // namespace Buttons
