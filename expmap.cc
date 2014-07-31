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
  struct ExpHydroMap
  {
    int32_t table[VideoConfig::height][VideoConfig::width];
    ExpHydroMap()
    {
      for (uintptr_t y = 0; y < VideoConfig::height; ++y) {
        for (uintptr_t x = 0; x < VideoConfig::width; ++x) {
          float center = (sin( float( x ) * M_PI * 2 * 3 / VideoConfig::width ) * 128 + 192);
          
          table[y][x] = int32_t( (16*std::max( ((center - y) / float(center - 0)), ((center - y) / float(center - 384)) ))*(1<<16) );
          // table[y][x] = int32_t( (exc*exc/1024)*(1<<16) );
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
  _action.biasedmotion( 16, hydro::motion( thm.table, _action ) );
}

Gate
ExpRoomBuf::start_incoming()
{
  return Gate( new ExpRoomBuf, Point<int32_t>(50, 50) );
}
