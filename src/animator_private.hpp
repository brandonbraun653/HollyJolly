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
#include <cstdint>
#include <cstring>
#include "ws2812.hpp"


/*-----------------------------------------------------------------------------
Literals
-----------------------------------------------------------------------------*/

/**
 * @brief Helper macro to declare a basic animation class conforming to the IAnimation interface
 */
#define DECLARE_ANIMATION_CLASS( name ) \
  class name : public IAnimation \
  { \
  public: \
    name(); \
    ~name(); \
    void initialize() final override; \
    void process() final override; \
    void stop() final override; \
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
    FADE,
    RAINBOW,
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
     * This is called periodically to update the animation state
     */
    virtual void process() = 0;

    /**
     * @brief Stops the animation, bringing it to an idle state.
     * Tear down any resources that were allocated during the initialization.
     */
    virtual void stop() = 0;
  };

  DECLARE_ANIMATION_CLASS( IdleAnimation );
  DECLARE_ANIMATION_CLASS( FullSweepColorBlock );

  /*---------------------------------------------------------------------------
  Private Functions
  ---------------------------------------------------------------------------*/

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
}  // namespace Animator

#endif  /* !HOLLY_JOLLY_INTERNAL_ANIMATOR_HPP */
