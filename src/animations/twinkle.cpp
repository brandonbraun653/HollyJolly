/******************************************************************************
 *  File Name:
 *    twinkle.cpp
 *
 *  Description:
 *    Implementation of the Twinkle animation
 *
 *  2024 | Brandon Braun | brandonbraun653@protonmail.com
 *****************************************************************************/

/*-----------------------------------------------------------------------------
Includes
-----------------------------------------------------------------------------*/
#include "animator_private.hpp"
#include "pico/time.h"
#include "holly_jolly_cfg.hpp"
#include <cstdlib>

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

  Twinkle::Twinkle()
  {
  }


  Twinkle::~Twinkle()
  {
  }


  void Twinkle::initialize()
  {
    led_idx       = 0;
    color         = 0;
    m_next_update = delayed_by_ms( get_absolute_time(), 500 );
  }


  bool Twinkle::process()
  {
    if( absolute_time_diff_us( get_absolute_time(), m_next_update ) > 0 )
    {
      return false;
    }

    m_next_update = delayed_by_ms( get_absolute_time(), 250 );

    uint32_t *p_render_buffer = LED::getRenderBuffer();
    memset( p_render_buffer, 0, sizeof( uint32_t ) * LED::count() );

    for( uint32_t i = 0; i < 10; i++ )
    {
      led_idx = rand() % LED::count();
      color   = COLOR_LIST[ rand() % COLOR_LIST_SIZE ];

      p_render_buffer[ led_idx ] = color;
    }

    return true;
  }


  void Twinkle::stop()
  {
  }
}    // namespace Animator
