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
  static constexpr float MIN_BRIGHTNESS  = 0.1f;
  static constexpr float BRIGHTNESS_STEP = 0.1f;

  /*---------------------------------------------------------------------------
  Static Data
  ---------------------------------------------------------------------------*/
  static IAnimation        *s_animations[ AnimationIndex::COUNT ];
  static volatile uint8_t   s_current_animation;
  static volatile float     s_global_brightness;

  /*---------------------------------------------------------------------------
  Static Function Declarations
  ---------------------------------------------------------------------------*/
  static IAnimation *get_current_animation();
  static void        scale_global_brightness();
  static void        on_button_bright_press();
  static void        on_button_action_press();

  /*---------------------------------------------------------------------------
  Public Functions
  ---------------------------------------------------------------------------*/

  void initialize()
  {
    /*-------------------------------------------------------------------------
    Initialize the static variables
    -------------------------------------------------------------------------*/
    memset( s_animations, 0, sizeof( IAnimation * ) * AnimationIndex::COUNT );
    s_current_animation = AnimationIndex::IDLE;
    s_global_brightness = 1.0f;

    /*-------------------------------------------------------------------------
    Bind the animations to their respective indexes
    -------------------------------------------------------------------------*/
    s_animations[ AnimationIndex::IDLE ]         = new IdleAnimation();
    s_animations[ AnimationIndex::COLOR_BLOCKS ] = new FullSweepColorBlock();
    s_animations[ AnimationIndex::TWINKLE ]      = new Twinkle();

    /*-------------------------------------------------------------------------
    Register the button callbacks
    -------------------------------------------------------------------------*/
    Buttons::onBrightKeyPress( on_button_bright_press );
    Buttons::onActionKeyPress( on_button_action_press );
  }


  void process()
  {
    bool draw_frame = false;

    /*-------------------------------------------------------------------------
    Process the current animation, drawing the next frame to the render buffer.
    -------------------------------------------------------------------------*/
    IAnimation *current = get_current_animation();
    if( current != nullptr )
    {
      draw_frame = current->process();
      scale_global_brightness();
    }

    /*-------------------------------------------------------------------------
    Swap the render buffers to display the new frame.
    -------------------------------------------------------------------------*/
    if( draw_frame )
    {
      LED::swapBuffers();
    }
  }


  void set_led_properties( uint32_t *const buffer, const uint32_t index, const uint32_t color, const float brightness )
  {
    uint8_t blue  = ( color & 0x00FF0000 ) >> 16;
    uint8_t red   = ( color & 0x0000FF00 ) >> 8;
    uint8_t green = ( color & 0x000000FF );

    red   = static_cast<uint8_t>( red * brightness );
    green = static_cast<uint8_t>( green * brightness );
    blue  = static_cast<uint8_t>( blue * brightness );

    buffer[ index ] = ( blue << 16 ) | ( red << 8 ) | green;
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
    IAnimation *current = nullptr;
    if( s_current_animation < AnimationIndex::COUNT )
    {
      current = s_animations[ s_current_animation ];
    }

    return current;
  }


  /**
   * @brief Scales the global brightness of the LED string
   */
  static void scale_global_brightness()
  {
    uint32_t *p_render_buffer = LED::getRenderBuffer();

    for( uint32_t i = 0; i < LED::count(); i++ )
    {
      uint32_t color = p_render_buffer[ i ];

      uint8_t blue  = ( color & 0x00FF0000 ) >> 16;
      uint8_t red   = ( color & 0x0000FF00 ) >> 8;
      uint8_t green = ( color & 0x000000FF );

      red   = static_cast<uint8_t>( red * s_global_brightness );
      green = static_cast<uint8_t>( green * s_global_brightness );
      blue  = static_cast<uint8_t>( blue * s_global_brightness );

      p_render_buffer[ i ] = ( blue << 16 ) | ( red << 8 ) | green;
    }
  }


  /**
   * @brief Updates the global brightness of the LED string
   */
  static void on_button_bright_press()
  {
    s_global_brightness += BRIGHTNESS_STEP;
    if( s_global_brightness >= MAX_BRIGHTNESS )
    {
      s_global_brightness = MIN_BRIGHTNESS;
    }

    /*-------------------------------------------------------------------------
    Clear out the buffer data to ensure a clean transition to the new
    brightness level.
    -------------------------------------------------------------------------*/
    LED::resetBuffers();
  }


  /**
   * @brief Switches to the next animation in the list
   */
  static void on_button_action_press()
  {
    /*-------------------------------------------------------------------------
    Stop the current animation and clear the render buffer
    -------------------------------------------------------------------------*/
    IAnimation *current = s_animations[ s_current_animation ];
    if( current != nullptr )
    {
      current->stop();
      memset( LED::getRenderBuffer(), 0, LED::count() * sizeof( uint32_t ) );
      LED::swapBuffers();
    }

    /*-------------------------------------------------------------------------
    Switch to the next animation in the list that isn't null
    -------------------------------------------------------------------------*/
    s_current_animation++;
    while( s_animations[ s_current_animation ] == nullptr )
    {
      s_current_animation++;
      if( s_current_animation >= AnimationIndex::COUNT )
      {
        s_current_animation = AnimationIndex::IDLE;
      }
    }

    s_animations[ s_current_animation ]->initialize();
  }

}    // namespace Animator
