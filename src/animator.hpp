/******************************************************************************
 *  File Name:
 *    animator.hpp
 *
 *  Description:
 *    Animation driver for the Holly Jolly project.
 *
 *  2024 | Brandon Braun | brandonbraun653@protonmail.com
 *****************************************************************************/

#pragma once
#ifndef HOLLY_JOLLY_ANIMATION_HPP
#define HOLLY_JOLLY_ANIMATION_HPP

/*-----------------------------------------------------------------------------
Includes
-----------------------------------------------------------------------------*/
#include <cstdint>

namespace Animator
{

  /**
   * @brief Clear the LED string by turning all LEDs off
   */
  void clear();

  /**
   * @brief Set the master brightess of Holly Jolly
   *
   * @param brightness  Value between 0.0 and 1.0
   */
  void setGlobalBrightness( const float brightness );

  /**
   * @brief Set the color of a specific LED in the string
   *
   * @param index    Index of the LED to change
   * @param red      Red component of the color
   * @param green    Green component of the color
   * @param blue     Blue component of the color
   */
  void setLedColor( const uint8_t index, const uint8_t red, const uint8_t green, const uint8_t blue );

  /**
   * @brief Set the color of all LEDs in the string
   *
   * @param red      Red component of the color
   * @param green    Green component of the color
   * @param blue     Blue component of the color
   */
  void setAllLedsColor( const uint8_t red, const uint8_t green, const uint8_t blue );

}    // namespace Animator

#endif /* !HOLLY_JOLLY_ANIMATION_HPP */
