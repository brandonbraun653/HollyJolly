/******************************************************************************
 *  File Name:
 *    animator_private.hpp
 *
 *  Description:
 *    Internal interface to the animation driver
 *
 *  2024 | Brandon Braun | brandonbraun653@protonmail.com
 *****************************************************************************/

#pragma once
#ifndef HOLLY_JOLLY_INTERNAL_ANIMATOR_HPP
#define HOLLY_JOLLY_INTERNAL_ANIMATOR_HPP

/*-----------------------------------------------------------------------------
Includes
-----------------------------------------------------------------------------*/
#include "pico/time.h"
#include "ws2812.hpp"
#include <cstdint>
#include <cstring>


/*-----------------------------------------------------------------------------
Literals
-----------------------------------------------------------------------------*/

/**
 * @brief Helper macro to declare a basic animation class conforming to the IAnimation interface
 */
#define DECLARE_ANIMATION_CLASS( name ) \
  class name : public IAnimation        \
  {                                     \
  public:                               \
    name();                             \
    ~name();                            \
    void initialize() final override;   \
    bool process() final override;      \
    void stop() final override;         \
                                        \
  protected:                            \
    absolute_time_t m_next_update;      \
  }

namespace Animator
{
  /*---------------------------------------------------------------------------
  Enumerations
  ---------------------------------------------------------------------------*/

  enum AnimationIndex : uint8_t
  {
    IDLE,
    COLOR_BLOCKS,
    TWINKLE,
    SOFT_GLOW,
    COUNT
  };

  /*---------------------------------------------------------------------------
  Private Classes
  ---------------------------------------------------------------------------*/

  /**
   * @brief Virtual interface to an animation object
   *
   * This class is used to define the interface for all animations that can be
   * run on the Holly Jolly project.
   */
  class IAnimation
  {
  public:
    virtual ~IAnimation() = default;

    /**
     * @brief Initialize the animation object.
     * Create any resources that are needed for the animation to run.
     */
    virtual void initialize() = 0;

    /**
     * @brief Process any animation updates
     * This is called periodically to update the animation state.
     * @return bool  True if a new frame was drawn, false otherwise
     */
    virtual bool process() = 0;

    /**
     * @brief Stops the animation, bringing it to an idle state.
     * Tear down any resources that were allocated during the initialization.
     */
    virtual void stop() = 0;
  };

  /* Make sure to add these animation classes to the initialize() method of animator.cpp */
  DECLARE_ANIMATION_CLASS( IdleAnimation );
  DECLARE_ANIMATION_CLASS( FullSweepColorBlock );
  DECLARE_ANIMATION_CLASS( Twinkle );
  DECLARE_ANIMATION_CLASS( SoftGlow );

  /*---------------------------------------------------------------------------
  Private Functions
  ---------------------------------------------------------------------------*/

  /**
   * @brief Set the brightness of a specific LED in the string
   *
   * @param buffer Backing memory for the LED strip
   * @param index Which LED to change
   * @param color Color to set the LED to
   * @param brightness Brightness level to set
   */
  void set_led_properties( uint32_t *const buffer, const uint32_t index, const uint32_t color, const float brightness );

}  // namespace Animator

#endif  /* !HOLLY_JOLLY_INTERNAL_ANIMATOR_HPP */
