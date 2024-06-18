/******************************************************************************
 *  File Name:
 *    idle.cpp
 *
 *  Description:
 *    Implementation of the idle animation
 *
 *  2024 | Brandon Braun | brandonbraun653@protonmail.com
 *****************************************************************************/

/*-----------------------------------------------------------------------------
Includes
-----------------------------------------------------------------------------*/
#include "animator_private.hpp"
#include "pico/time.h"

namespace Animator
{
  /*---------------------------------------------------------------------------
  Static Data
  ---------------------------------------------------------------------------*/

  static uint32_t led_idx;
  static uint32_t color;

  /*---------------------------------------------------------------------------
  Idle Animation Class
  ---------------------------------------------------------------------------*/

  IdleAnimation::IdleAnimation()
  {
  }


  IdleAnimation::~IdleAnimation()
  {
  }


  void IdleAnimation::initialize()
  {
    led_idx       = 0;
    color         = 0;
    m_next_update = delayed_by_ms( get_absolute_time(), 500 );
  }


  bool IdleAnimation::process()
  {
    if( absolute_time_diff_us( get_absolute_time(), m_next_update ) > 0 )
    {
      return false;
    }

    m_next_update = delayed_by_ms( get_absolute_time(), 100 );

    uint32_t *p_render_buffer = LED::getRenderBuffer();
    memset( p_render_buffer, 0, sizeof( uint32_t ) * LED::count() );

    switch( color )
    {
      case 0:
        p_render_buffer[ led_idx ] = 0x110000;    // b
        color++;
        break;
      case 1:
        p_render_buffer[ led_idx ] = 0x001100;    // r
        color++;
        break;
      case 2:
        p_render_buffer[ led_idx ] = 0x00000A;    // g
        color                      = 0;
        break;
    }

    led_idx = ( led_idx + 1 ) % LED::count();
    return true;
  }


  void IdleAnimation::stop()
  {
  }
}    // namespace Animator
