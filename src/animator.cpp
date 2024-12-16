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
  static IAnimation      *s_animators[ AnimationIndex::COUNT ];
  static volatile uint8_t s_animation_idx;
  static volatile float   s_global_brightness;

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
    memset( s_animators, 0, sizeof( IAnimation * ) * AnimationIndex::COUNT );
    s_animation_idx = AnimationIndex::IDLE;
    s_global_brightness = 0.2f;

    /*-------------------------------------------------------------------------
    Bind the animations to their respective indexes
    -------------------------------------------------------------------------*/
    s_animators[ AnimationIndex::IDLE ]         = new IdleAnimation();
    s_animators[ AnimationIndex::COLOR_BLOCKS ] = new FullSweepColorBlock();
    s_animators[ AnimationIndex::TWINKLE ]      = new Twinkle();
    s_animators[ AnimationIndex::SOFT_GLOW ]    = new SoftGlow();

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
    uint8_t blue  = ( color & LED::WS2812_BLUE_MSK ) >> 16;
    uint8_t red   = ( color & LED::WS2812_RED_MSK ) >> 8;
    uint8_t green = ( color & LED::WS2812_GREEN_MSK );

    red   = static_cast<uint8_t>( red * brightness );
    green = static_cast<uint8_t>( green * brightness );
    blue  = static_cast<uint8_t>( blue * brightness );

    buffer[ index ] = ( ( blue << 16 ) | ( red << 8 ) | green ) & LED::WS2812_DATA_MSK;
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
    if( s_animation_idx < AnimationIndex::COUNT )
    {
      current = s_animators[ s_animation_idx ];
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

      uint8_t blue  = ( color & LED::WS2812_BLUE_MSK ) >> 16;
      uint8_t red   = ( color & LED::WS2812_RED_MSK ) >> 8;
      uint8_t green = ( color & LED::WS2812_GREEN_MSK );

      red   = static_cast<uint8_t>( red * s_global_brightness );
      green = static_cast<uint8_t>( green * s_global_brightness );
      blue  = static_cast<uint8_t>( blue * s_global_brightness );

      p_render_buffer[ i ] = ( ( blue << 16 ) | ( red << 8 ) | green ) & LED::WS2812_DATA_MSK;
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
    IAnimation *current = s_animators[ s_animation_idx ];
    if( current != nullptr )
    {
      current->stop();
      memset( LED::getRenderBuffer(), 0, LED::count() * sizeof( uint32_t ) );
      LED::swapBuffers();
    }

    /*-------------------------------------------------------------------------
    Switch to the next animation in the list
    -------------------------------------------------------------------------*/
    s_animation_idx = ( s_animation_idx + 1 ) % AnimationIndex::COUNT;
    s_animators[ s_animation_idx ]->initialize();
  }

}    // namespace Animator
