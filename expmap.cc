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
    hydro::Delay table[2][VideoConfig::height][VideoConfig::width];
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
          table[0][y][x].set( normal + radial );
        }
      }
      for (uintptr_t y = 0; y < VideoConfig::height; ++y) {
        for (uintptr_t x = 0; x < VideoConfig::width; ++x) {
          if ((y < 32) or (y > (VideoConfig::height-32))) { table[1][y][x].set( nan("") ); continue; }
          if ((x < 32) or (x > (VideoConfig::width-32))) { table[1][y][x].set( nan("") ); continue; }
          float center = (sin( float( x ) * M_PI * 2 * 3 / VideoConfig::width ) * 128 + 192);
          table[1][y][x].set( 16 * std::max( ((center - y) / float(center - 32)), ((center - y) / float(center - (VideoConfig::height-32))) ) );
        }
      }
    }
  } thm;
};

struct ExpRoomBuf : public virtual RoomBuf
{
  explicit ExpRoomBuf( uintptr_t _idx ) : m_index(_idx) {}
  int cmp( RoomBuf const& _rb ) const { return tgcmp( m_index, dynamic_cast<ExpRoomBuf const&>( _rb ).m_index ); }
  std::string getname() const { std::ostringstream oss; oss << "ExpRoom[" << m_index << "]"; return oss.str(); }
  
  void
  process( Action& _action ) const
  {
    if ((m_index & -2) == 0) {
      _action.blit( gallery::classic_bg );
      hydro::effect( thm.table[m_index], _action );
      _action.normalmotion();
      _action.moremotion( hydro::motion( thm.table[m_index], _action ) );
    } else if (m_index == 4) {
      
    }
  }

  uintptr_t m_index;
};

Gate ExpMap::start_incoming() { return Gate( new ExpRoomBuf(1), Point<int32_t>(50, 50) ); }
