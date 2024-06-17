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
#include "pico/time.h"
#include "pico/types.h"
#include <cstdint>

namespace Buttons
{
  /*---------------------------------------------------------------------------
  Constants
  ---------------------------------------------------------------------------*/

  static constexpr uint     s_pin_input_bright = 19;
  static constexpr uint     s_pin_input_action = 25;
  static constexpr uint32_t s_debounce_ms      = 50;

  /*---------------------------------------------------------------------------
  Static Variables
  ---------------------------------------------------------------------------*/

  static volatile ButtonCallback s_bright_press_cb;
  static volatile ButtonCallback s_action_press_cb;
  static volatile uint32_t       s_last_bright_press_time;
  static volatile uint32_t       s_last_action_press_time;
  static volatile bool           s_pending_bright_press;
  static volatile bool           s_pending_action_press;

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
    /*-------------------------------------------------------------------------
    Only process falling edge events. Per the hardware schematic, the buttons
    are active low.
    -------------------------------------------------------------------------*/
    if( ( event_mask & GPIO_IRQ_EDGE_FALL ) != GPIO_IRQ_EDGE_FALL )
    {
      return;
    }

    /*-------------------------------------------------------------------------
    Determine which button was pressed and record the time. Actual processing
    will occur in the main loop.
    -------------------------------------------------------------------------*/
    if( ( gpio == s_pin_input_bright ) && !s_pending_bright_press )
    {
      s_pending_bright_press   = true;
      s_last_bright_press_time = to_ms_since_boot( get_absolute_time() );
    }
    else if( ( gpio == s_pin_input_action ) && !s_pending_action_press )
    {
      s_pending_action_press   = true;
      s_last_action_press_time = to_ms_since_boot( get_absolute_time() );
    }
  }

  /*---------------------------------------------------------------------------
  Public Functions
  ---------------------------------------------------------------------------*/

  void initialize()
  {
    /*-------------------------------------------------------------------------
    Initialize the static variables
    -------------------------------------------------------------------------*/
    s_bright_press_cb        = nullptr;
    s_action_press_cb        = nullptr;
    s_last_bright_press_time = 0;
    s_last_action_press_time = 0;
    s_pending_bright_press   = false;
    s_pending_action_press   = false;

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


  void process()
  {
    const uint32_t current_time = to_ms_since_boot( get_absolute_time() );

    /*-------------------------------------------------------------------------
    Process the brightness control input
    -------------------------------------------------------------------------*/
    if( s_pending_bright_press && ( ( current_time - s_last_bright_press_time ) >= s_debounce_ms ) )
    {
      if( s_bright_press_cb && ( gpio_get( s_pin_input_bright ) == 0 ) )
      {
        s_bright_press_cb();
      }

      s_pending_bright_press = false;
    }

    /*-------------------------------------------------------------------------
    Process the action control input
    -------------------------------------------------------------------------*/
    if( s_pending_action_press && ( ( current_time - s_last_action_press_time ) >= s_debounce_ms ) )
    {
      if( s_action_press_cb && ( gpio_get( s_pin_input_action ) == 0 ) )
      {
        s_action_press_cb();
      }

      s_pending_action_press = false;
    }
  }


  void onBrightKeyPress( ButtonCallback callback )
  {
    s_bright_press_cb = callback;
  }


  void onActionKeyPress( ButtonCallback callback )
  {
    s_action_press_cb = callback;
  }

}    // namespace Buttons
