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
#include "animator_private.hpp"
#include "buttons.hpp"
#include "ws2812.hpp"
#include "pico/critical_section.h"

namespace Animator
{
  /*---------------------------------------------------------------------------
  Constants
  ---------------------------------------------------------------------------*/

  static constexpr float MAX_BRIGHTNESS  = 1.0f;
  static constexpr float MIN_BRIGHTNESS  = 0.0f;
  static constexpr float BRIGHTNESS_STEP = 0.1f;

  /*---------------------------------------------------------------------------
  Static Data
  ---------------------------------------------------------------------------*/
  static IAnimation        *s_animations[ AnimationIndex::COUNT ];
  static critical_section_t s_critical_section;
  static volatile uint8_t   s_current_animation;
  static volatile float     s_global_brightness;

  /*---------------------------------------------------------------------------
  Static Function Declarations
  ---------------------------------------------------------------------------*/
  static IAnimation *get_current_animation();
  static void        scale_global_brightness();
  static void        isr_on_button_bright_press();
  static void        isr_on_button_action_press();

  /*---------------------------------------------------------------------------
  Public Functions
  ---------------------------------------------------------------------------*/

  void initialize()
  {
    // Configure module memory
    critical_section_init( &s_critical_section );

    // Initialize all the animations
    s_current_animation = AnimationIndex::IDLE;
    s_global_brightness = 1.0f;

    memset( s_animations, 0, sizeof( IAnimation * ) * AnimationIndex::COUNT );
    s_animations[ AnimationIndex::IDLE ] = new IdleAnimation();
    s_animations[ AnimationIndex::COLOR_BLOCKS ] = new FullSweepColorBlock();

    // Register the button callbacks last so that the animations are fully
    // initialized before they start receiving input.
    Buttons::onBrightKeyPress( isr_on_button_bright_press );
    Buttons::onActionKeyPress( isr_on_button_action_press );
  }


  void process()
  {
    // Process the current animation. This will draw a new frame to the render buffer.
    IAnimation *current = get_current_animation();
    if( current != nullptr )
    {
      current->process();
      scale_global_brightness();
    }

    // Apply the frame to the LED string
    LED::swapBuffers();
  }

  void clear()
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

  /**
   * @brief Safely retrieves the current animation object.
   *
   * Animations are updated inside an ISR context, so the current selection
   * can be changed at any time. This function ensures that the current
   * animation is retrieved in a ISR safe manner.
   *
   * @return IAnimation*
   */
  static IAnimation *get_current_animation()
  {
    critical_section_enter_blocking( &s_critical_section );

    IAnimation *current = nullptr;
    if( s_current_animation < AnimationIndex::COUNT )
    {
      current = s_animations[ s_current_animation ];
    }

    critical_section_exit( &s_critical_section );
    return current;
  }


  /**
   * @brief Scales the global brightness of the LED string
   */
  static void scale_global_brightness()
  {
    // This is updated in an ISR context, so we need to make a copy
    const float brightness_copy = s_global_brightness;

    // Apply the brightness scaling to the current render buffer
    uint32_t *p_render_buffer = LED::getRenderBuffer();
    for( uint32_t i = 0; i < LED::count(); i++ )
    {
      uint32_t color = p_render_buffer[ i ];

      uint8_t blue  = ( color & 0x00FF0000 ) >> 16;
      uint8_t red   = ( color & 0x0000FF00 ) >> 8;
      uint8_t green = ( color & 0x000000FF );

      red   = static_cast<uint8_t>( red * brightness_copy );
      green = static_cast<uint8_t>( green * brightness_copy );
      blue  = static_cast<uint8_t>( blue * brightness_copy );

      p_render_buffer[ i ] = ( blue << 16 ) | ( red << 8 ) | green;
    }
  }


  /**
   * @brief Updates the global brightness of the LED string
   */
  static void isr_on_button_bright_press()
  {
    s_global_brightness += BRIGHTNESS_STEP;
    if( s_global_brightness >= MAX_BRIGHTNESS )
    {
      s_global_brightness = MIN_BRIGHTNESS;
    }
  }


  /**
   * @brief Switches to the next animation in the list
   */
  static void isr_on_button_action_press()
  {
    // Stop the old animation
    IAnimation *current = s_animations[ s_current_animation ];
    if( current != nullptr )
    {
      current->stop();
      clear();
      LED::swapBuffers();
    }

    // Move to the next animation
    s_current_animation++;
    if( s_current_animation >= AnimationIndex::COUNT )
    {
      s_current_animation = AnimationIndex::IDLE;
    }

    // Initialize the new animation
    IAnimation *next = s_animations[ s_current_animation ];
    if( next != nullptr )
    {
      next->initialize();
    }
  }

}    // namespace Animator
