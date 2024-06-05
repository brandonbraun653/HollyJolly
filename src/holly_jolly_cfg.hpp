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

/**
 * @brief Periodic refresh rate of the animation system
 *
 * The animation system will draw a new frame at this rate, then
 * flush the frame buffer to the LED strip.
 */
static constexpr uint32_t FRAME_REFRESH_RATE_MS = 10;

#endif  /* !HOLLY_JOLLY_CONFIG_HPP_HPP */
