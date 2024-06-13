/******************************************************************************
 *  File Name:
 *    full_sweep_color_block.cpp
 *
 *  Description:
 *    Alternates the color of the entire strip in a sweeping pattern
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
  static uint32_t        color;
  static absolute_time_t next_update;

  FullSweepColorBlock::FullSweepColorBlock()
  {
  }


  FullSweepColorBlock::~FullSweepColorBlock()
  {
  }


  void FullSweepColorBlock::initialize()
  {
    color       = 0;
    next_update = make_timeout_time_us( 100'000 );
  }


  void FullSweepColorBlock::process()
  {
    if( !time_reached( next_update ) )
    {
      return;
    }

    next_update = make_timeout_time_us( 100'000 );

    uint32_t next_color = 0;
    switch( color )
    {
      case 0:
        next_color = 0x110000;    // b
        color++;
        break;
      case 1:
        next_color = 0x001100;    // r
        color++;
        break;
      case 2:
        next_color = 0x00000A;    // g
        color      = 0;
        break;
    }

    uint32_t *p_render_buffer = LED::getRenderBuffer();
    memset( p_render_buffer, next_color, sizeof( uint32_t ) * LED::count() );
  }


  void FullSweepColorBlock::stop()
  {
  }

}  // namespace Animator
