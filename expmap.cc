#include <expmap.hh>
#include <action.hh>
#include <video.hh>
#include <hydro.hh>
#include <SDL/SDL.h>
#include <iostream>
#include <sstream>
#include <gallery.hh>
#include <cmath>

namespace {
  // struct ExpHydroMap
  // {
  //   hydro::Delay table[VideoConfig::height][VideoConfig::width];
  //   ExpHydroMap()
  //   {
  //     for (uintptr_t y = 0; y < VideoConfig::height; ++y) {
  //       for (uintptr_t x = 0; x < VideoConfig::width; ++x) {
  //         float center = (sin( float( x ) * M_PI * 2 * 3 / VideoConfig::width ) * 128 + 192);
          
  //         table[y][x].set( 16 * std::max( ((center - y) / float(center - 0)), ((center - y) / float(center - 384)) ) );
  //       }
  //     }
  //   }
  // } thm;
  
  struct ExpHydroMap
  {
    hydro::Delay table[VideoConfig::height][VideoConfig::width];
    ExpHydroMap()
    {
      for (uintptr_t y = 0; y < VideoConfig::height; ++y) {
        for (uintptr_t x = 0; x < VideoConfig::width; ++x) {
          Point<float> pos( x-0.5*(VideoConfig::width-1), y-0.5*(VideoConfig::height-1) );
          double sqnorm = pos.sqnorm();
          double norm = sqrt( sqnorm );
          // double radial = sqnorm < 160*160 ? (sqnorm*norm/65536) : nan("");
          double radial =
            (norm < 128) ? ((norm-128)*norm/2048) :
            (norm < 160) ? (((norm-128)*(norm-128) + 128)*(norm-128)/2048) :
            nan("");
          
          double angle = (atan2(pos.m_y, pos.m_x)+M_PI);
          double normal = angle*64/M_PI;
          table[y][x].set( normal + radial );
        }
      }
    }
  } thm;
};

void
ExpRoomBuf::process( Action& _action ) const
{
  // Scene Draw
  _action.blit( gallery::classic_bg );
  
  hydro::effect( thm.table, _action );
  _action.biasedmotion( hydro::motion( thm.table, _action ) );
}

Gate
ExpRoomBuf::start_incoming()
{
  return Gate( new ExpRoomBuf, Point<int32_t>(50, 50) );
}
