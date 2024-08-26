/******************************************************************************
 *  File Name:
 *    holly_jolly_cfg.hpp
 *
 *  Description:
 *    Configuration options for Holly Jolly
 *
 *  2024 | Brandon Braun | brandonbraun653@protonmail.com
 *****************************************************************************/

#pragma once
#ifndef HOLLY_JOLLY_CONFIG_HPP
#define HOLLY_JOLLY_CONFIG_HPP

/*-----------------------------------------------------------------------------
Includes
-----------------------------------------------------------------------------*/
#include <cstdint>

/*-----------------------------------------------------------------------------
Constants
-----------------------------------------------------------------------------*/

static constexpr uint32_t COLOR_RED     = 0x0000FF;
static constexpr uint32_t COLOR_GREEN   = 0x00FF00;
static constexpr uint32_t COLOR_BLUE    = 0xFF0000;
static constexpr uint32_t COLOR_YELLOW  = 0x00FFFF;
static constexpr uint32_t COLOR_MAGENTA = 0xFF00FF;
static constexpr uint32_t COLOR_CYAN    = 0xFFFF00;
static constexpr uint32_t COLOR_ORANGE  = 0xFF8000;
static constexpr uint32_t COLOR_PURPLE  = 0x8000FF;
static constexpr uint32_t COLOR_LIME    = 0x00FF80;
static constexpr uint32_t COLOR_PINK    = 0xFF0080;

static constexpr uint32_t COLOR_LIST[] =
{
  COLOR_RED,
  COLOR_GREEN,
  COLOR_BLUE,
  COLOR_YELLOW,
  COLOR_MAGENTA,
  COLOR_CYAN,
  COLOR_ORANGE,
  COLOR_PURPLE,
  COLOR_LIME,
  COLOR_PINK
};

static constexpr uint32_t COLOR_LIST_SIZE = sizeof( COLOR_LIST ) / sizeof( COLOR_LIST[ 0 ] );

/**
 * @brief Periodic refresh rate of the animation system
 *
 * The animation system will draw a new frame at this rate, then
 * flush the frame buffer to the LED strip.
 */
static constexpr uint32_t FRAME_REFRESH_RATE_MS = 10;

#endif  /* !HOLLY_JOLLY_CONFIG_HPP_HPP */
