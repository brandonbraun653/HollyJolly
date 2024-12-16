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
  Constants
  ---------------------------------------------------------------------------*/

  static constexpr uint32_t WS2812_NUM_LEDS  = 32;            // Number of LEDs in the string
  static constexpr uint32_t WS2812_BLUE_MSK  = 0x00FF0000;    // Bitmask for the blue channel
  static constexpr uint32_t WS2812_RED_MSK   = 0x0000FF00;    // Bitmask for the red channel
  static constexpr uint32_t WS2812_GREEN_MSK = 0x000000FF;    // Bitmask for the green channel
  static constexpr uint32_t WS2812_DATA_MSK  = 0x00FFFFFF;    // Bitmask for all color data

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
   * @return uint
   */
  static constexpr uint32_t count()
  {
    return WS2812_NUM_LEDS;
  }

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
   * @brief Get a read-only pointer to the current display buffer
   *
   * See getRenderBuffer() for more information on the buffer format.
   * @return const uint32_t*
   */
  const uint32_t *getDisplayBuffer();

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
