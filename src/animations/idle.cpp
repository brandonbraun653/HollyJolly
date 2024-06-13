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

  static uint32_t        led_idx;
  static uint32_t        color;
  static absolute_time_t next_update;

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
    led_idx     = 0;
    color       = 0;
    next_update = make_timeout_time_us( 100'000 );
  }


  void IdleAnimation::process()
  {
    if( !time_reached( next_update ) )
    {
      return;
    }

    next_update = make_timeout_time_us( 100'000 );

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
  }


  void IdleAnimation::stop()
  {
  }
}    // namespace Animator
