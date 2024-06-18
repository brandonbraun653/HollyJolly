/******************************************************************************
 *  File Name:
 *    ws2812.hpp
 *
 *  Description:
 *    Driver for interacting with the raw WS2812 LED strip on the board. This
 *    provides an interface for interacting with the frame buffers.
 *
 *  2024 | Brandon Braun | brandonbraun653@protonmail.com
 *****************************************************************************/

#pragma once
#ifndef HOLLY_JOLLY_WS2812_HPP
#define HOLLY_JOLLY_WS2812_HPP

/*-----------------------------------------------------------------------------
Includes
-----------------------------------------------------------------------------*/
#include <cstdint>

namespace LED
{
  /*---------------------------------------------------------------------------
  Public Functions
  ---------------------------------------------------------------------------*/

  /**
   * @brief Initialize the hardware to drive the WS2812 LEDs.
   *
   * This configures the PIO to update the LED array with DMA transfers.
   */
  void initialize();

  /**
   * @brief Total number of LEDs in the string
   * @return uint32_t
   */
  uint32_t count();

  /**
   * @brief Get a pointer to the current render buffer
   *
   * This buffer is sized to hold a single 32-bit color value for each LED in the string.
   * The total number of LEDs can be queried with the count() function.
   *
   * The data format for proper display is 0x00BBRRGG.
   *
   * @return uint32_t*
   */
  uint32_t *getRenderBuffer();

  /**
   * @brief Swap the render buffer with the display buffer.
   *
   * This will cause the new display buffer to be rendered to the LEDs.
   */
  void swapBuffers();

  /**
   * @brief Resets both the render and display buffers to all zeros.
   */
  void resetBuffers();

}    // namespace LED

#endif /* !HOLLY_JOLLY_WS2812_HPP */
