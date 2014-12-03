#include <expmap.hh>
#include <action.hh>
#include <video.hh>
#include <hydro.hh>
#include <iostream>
#include <sstream>
#include <gallery.hh>
#include <cmath>

namespace {
  struct ExpHydroMap
  {
    hydro::Delay table[2][Screen::height][Screen::width];
    ExpHydroMap()
    {
      for (uintptr_t y = 0; y < Screen::height; ++y) {
        for (uintptr_t x = 0; x < Screen::width; ++x) {
          Point<float> pos( x-0.5*(Screen::width-1), y-0.5*(Screen::height-1) );
          double sqnorm = pos.sqnorm();
          double norm = sqrt( sqnorm );
          // double radial = sqnorm < 160*160 ? (sqnorm*norm/65536) : nan("");
          double radial =
            (norm < 128) ? ((norm-128)*norm/2048) :
            (norm < 160) ? (((norm-128)*(norm-128) + 128)*(norm-128)/2048) :
            nan("");
          
          double angle = (atan2(pos.y, pos.x)+M_PI);
          double normal = angle*64/M_PI;
          table[0][y][x].set( normal + radial );
        }
      }
      for (uintptr_t y = 0; y < Screen::height; ++y) {
        for (uintptr_t x = 0; x < Screen::width; ++x) {
          if ((y < 32) or (y > (Screen::height-32))) { table[1][y][x].set( nan("") ); continue; }
          if ((x < 32) or (x > (Screen::width-32))) { table[1][y][x].set( nan("") ); continue; }
          float center = (sin( float( x ) * M_PI * 2 * 3 / Screen::width ) * 128 + 192);
          table[1][y][x].set( 16 * std::max( ((center - y) / float(center - 32)), ((center - y) / float(center - (Screen::height-32))) ) );
        }
      }
    }
  } thm;

  struct ExpRoomBuf : public virtual RoomBuf
  {
    explicit ExpRoomBuf( uintptr_t _idx ) : m_index(_idx) {}
    int cmp( RoomBuf const& _rb ) const { return tgcmp( m_index, dynamic_cast<ExpRoomBuf const&>( _rb ).m_index ); }
    std::string getname() const { std::ostringstream oss; oss << "ExpRoom[" << m_index << "]"; return oss.str(); }
  
    void
    process( Action& _action ) const
    {
      if ((m_index & -2) == 0) {
        _action.cornerblit( Point<int32_t>(), gallery::classic_bg );
        hydro::effect( thm.table[m_index], _action );
        _action.normalmotion();
        _action.moremotion( hydro::motion( thm.table[m_index], _action ) );
      } else if (m_index == 4) {
        Screen::pixels_t bg;
        for (int32_t y = 0; y < (int32_t)pixheight( bg ); ++y ) {
          for (int32_t x = 0; x < (int32_t)pixwidth( bg ); ++x ) {
            if (y < 200) {
              bg[y][x].set( 0xc0, 0x80, 0x80, 0xff );
            }
            else {
              bg[y][x].set( 0x80, 0xc0, 0x80, 0xff );
            }
          }
        }
        _action.cornerblit( Point<int32_t>(), bg );
        Pixel dot[9][9];
        for (uintptr_t y = 0; y < pixheight( dot ); ++y) {
          for (uintptr_t x = 0; x < pixheight( dot ); ++x) {
            uint8_t color = uint8_t( std::max( 255. - 8*((y-4)*(y-4)+(x-4)*(x-4)), 0. ) );
            dot[y][x].set( color, color, color, color );
          }
        }
        Point<float> tail = (_action.pos() - _action.motion()*2);
        _action.centerblit( tail.rebind<int32_t>(), dot );
        if (_action.pos().y < 200) {
          _action.moremotion( _action.motion() - Point<float>( 0, -1 ) );
        } else {
          _action.normalmotion();
        }
        
      }
    }

    uintptr_t m_index;
  };

};

Gate ExpMap::start_incoming() { return Gate( new ExpRoomBuf(4), Point<int32_t>(50, 50) ); }
