/******************************************************************************
 *  File Name:
 *    animator.cpp
 *
 *  Description:
 *    Animation driver for the Holly Jolly project.
 *
 *  2024 | Brandon Braun | brandonbraun653@protonmail.com
 *****************************************************************************/

/*-----------------------------------------------------------------------------
Includes
-----------------------------------------------------------------------------*/
#include "animator.hpp"
#include "buttons.hpp"

namespace Animator
{
  /*---------------------------------------------------------------------------
  Static Function Declarations
  ---------------------------------------------------------------------------*/

  static void on_button_bright_press();
  static void on_button_action_press();

  /*---------------------------------------------------------------------------
  Public Functions
  ---------------------------------------------------------------------------*/

  void initialize()
  {
    Buttons::onBrightKeyPress( on_button_bright_press );
    Buttons::onActionKeyPress( on_button_action_press );
  }


  void process()
  {

  }

  void clear()
  {
  }

  void setGlobalBrightness( const float brightness )
  {
  }

  void setLedColor( const uint8_t index, const uint8_t red, const uint8_t green, const uint8_t blue )
  {
  }

  void setAllLedsColor( const uint8_t red, const uint8_t green, const uint8_t blue )
  {
  }

  /*---------------------------------------------------------------------------
  Static Function Implementations
  ---------------------------------------------------------------------------*/

  static void on_button_bright_press()
  {
  }


  static void on_button_action_press()
  {
  }

}  // namespace Animator
