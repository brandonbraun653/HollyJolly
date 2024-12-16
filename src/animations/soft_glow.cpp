/******************************************************************************
 *  File Name:
 *    soft_glow.cpp
 *
 *  Description:
 *    Animation that creates a soft glow effect on the tree, randomizing the
 *    color of each LED and then slowly fading it out.
 *
 *  2024 | Brandon Braun | brandonbraun653@protonmail.com
 *****************************************************************************/

/*-----------------------------------------------------------------------------
Includes
-----------------------------------------------------------------------------*/
#include "animator_private.hpp"
#include "pico/time.h"
#include "ws2812.hpp"
#include <cstdlib>

namespace Animator
{
  /*---------------------------------------------------------------------------
  Structures
  ---------------------------------------------------------------------------*/

  struct LedState
  {
    uint32_t color;
    uint32_t fade;
    uint32_t fade_rate;
    bool fading_out;  // New field to track fade direction
  };

  /*---------------------------------------------------------------------------
  Static Data
  ---------------------------------------------------------------------------*/

  static LedState s_led_states[ LED::count() ];

  /*---------------------------------------------------------------------------
  Soft Glow Animation Class
  ---------------------------------------------------------------------------*/

  SoftGlow::SoftGlow()
  {
  }


  SoftGlow::~SoftGlow()
  {
  }


  void SoftGlow::initialize()
  {
    /*-------------------------------------------------------------------------
    Randomize the initial state of each LED
    -------------------------------------------------------------------------*/
    for( uint32_t i = 0; i < LED::count(); i++ )
    {
      s_led_states[ i ].color     = rand();
      s_led_states[ i ].fade      = rand() % 256;  // Random starting fade value
      s_led_states[ i ].fade_rate = (rand() % 5) + 1;  // Random fade rate between 1 and 5
      s_led_states[ i ].fading_out = (rand() % 2) == 0;  // Randomize fade direction
    }

    m_next_update = delayed_by_ms( get_absolute_time(), 500 );
  }


  bool SoftGlow::process()
  {
    if( absolute_time_diff_us( get_absolute_time(), m_next_update ) > 0 )
    {
      return false;
    }

    m_next_update = delayed_by_ms( get_absolute_time(), 25 );

    /*-------------------------------------------------------------------------
    Copy the current display buffer to the render buffer
    -------------------------------------------------------------------------*/
    auto p_render_buffer = LED::getRenderBuffer();
    auto p_display_buffer = LED::getDisplayBuffer();
    memcpy( p_render_buffer, p_display_buffer, sizeof( uint32_t ) * LED::count() );

    /*-------------------------------------------------------------------------
    Tweak the color of each LED and fade it in or out
    -------------------------------------------------------------------------*/
    for( uint32_t i = 0; i < LED::count(); i++ )
    {
      auto &led = s_led_states[ i ];

      /*-----------------------------------------------------------------------
      Randomize the color of the LED
      -----------------------------------------------------------------------*/
      if( led.fade == 0 )
      {
        led.color = rand();
      }

      /*-----------------------------------------------------------------------
      Adjust the color of the LED
      -----------------------------------------------------------------------*/
      uint8_t red   = ( led.color & LED::WS2812_RED_MSK ) >> 8;
      uint8_t green = ( led.color & LED::WS2812_GREEN_MSK );
      uint8_t blue  = ( led.color & LED::WS2812_BLUE_MSK ) >> 16;

      red   = static_cast<uint8_t>( red * led.fade / 255 );
      green = static_cast<uint8_t>( green * led.fade / 255 );
      blue  = static_cast<uint8_t>( blue * led.fade / 255 );

      p_render_buffer[ i ] = ( ( blue << 16 ) | ( red << 8 ) | green ) & LED::WS2812_DATA_MSK;

      /*-----------------------------------------------------------------------
      Update the fade state
      -----------------------------------------------------------------------*/
      if( led.fading_out )
      {
        if( led.fade > led.fade_rate )
        {
          led.fade -= led.fade_rate;
        }
        else
        {
          led.fade = 0;
          led.fading_out = false;  // Switch to fading in
        }
      }
      else
      {
        if( led.fade < 255 - led.fade_rate )
        {
          led.fade += led.fade_rate;
        }
        else
        {
          led.fade = 255;
          led.fading_out = true;  // Switch to fading out
        }
      }
    }

    return true;
  }


  void SoftGlow::stop()
  {
  }

}  // namespace Animator
