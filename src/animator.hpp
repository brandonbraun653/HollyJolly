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

namespace Animator
{
  /*---------------------------------------------------------------------------
  Public Functions
  ---------------------------------------------------------------------------*/

  /**
   * @brief Initialize the animation system
   */
  void initialize();

  /**
   * @brief Update the current animation state
   */
  void process();

}    // namespace Animator

#endif /* !HOLLY_JOLLY_ANIMATION_HPP */
