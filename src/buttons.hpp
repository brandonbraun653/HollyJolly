/******************************************************************************
 *  File Name:
 *    buttons.hpp
 *
 *  Description:
 *    Driver for the physical buttons on the Holly Jolly
 *
 *  2024 | Brandon Braun | brandonbraun653@protonmail.com
 *****************************************************************************/

#pragma once
#ifndef HOLLY_JOLLY_BUTTONS_HPP
#define HOLLY_JOLLY_BUTTONS_HPP

/*-----------------------------------------------------------------------------
Includes
-----------------------------------------------------------------------------*/
#include <cstdint>

namespace Buttons
{
  /*---------------------------------------------------------------------------
  Aliases
  ---------------------------------------------------------------------------*/

  using ButtonCallback = void ( * )( void );

  /*---------------------------------------------------------------------------
  Public Functions
  ---------------------------------------------------------------------------*/

  /**
   * @brief Initializes the system to read button inputs
   */
  void initialize();

  /**
   * @brief Periodic processing of button events
   */
  void process();

  /**
   * @brief Registers a callback for when the brightness button is pressed
   *
   * @param callback  Function to call when the button is pressed
   */
  void onBrightKeyPress( ButtonCallback callback );

  /**
   * @brief Registers a callback for when the action button is pressed
   *
   * @param callback  Function to call when the button is pressed
   */
  void onActionKeyPress( ButtonCallback callback );

}    // namespace Buttons

#endif /* !HOLLY_JOLLY_BUTTONS_HPP */
