#ifndef __HYDRO_HH__
#define __HYDRO_HH__

#include <video.hh>
#include <timeline.hh>
#include <SDL/SDL.h>
#include <cmath>
#include <inttypes.h>

struct SDL_Surface;

namespace hydro
{

  template <typename mapT>
  bool grad( mapT const& _table, Point<int32_t> const& pos, Point<float>& _grad )
  {
    if ((pos.m_y<=0) or (pos.m_y>=(Screen::height-1)) or (pos.m_x<=0) or (pos.m_x>=(Screen::width-1))) return false;
    int32_t vx0; if ((vx0 = _table[pos.m_y][pos.m_x-1].value) & 0x80000000) return false;
    int32_t vx1; if ((vx1 = _table[pos.m_y][pos.m_x+1].value) & 0x80000000) return false;
    int32_t vy0; if ((vy0 = _table[pos.m_y-1][pos.m_x].value) & 0x80000000) return false;
    int32_t vy1; if ((vy1 = _table[pos.m_y+1][pos.m_x].value) & 0x80000000) return false;
    _grad.m_x = double( (vx1-vx0) << 1 ) / 2 / double( 1<<29 );
    _grad.m_y = double( (vy1-vy0) << 1 ) / 2 / double( 1<<29 );
    return true;
  }


  template <typename mapT, typename actionT>
  void effect( mapT const& _table, actionT& _action )
  {
    screen_t& screen = _action.thescreen.pixels;
    date_t date = _action.now();
    for (int y = 0, ystop = pixheight(screen); y < ystop; ++y) {
      for (int x = 0, xstop = pixwidth(screen); x < xstop; ++x) {
        int32_t value = _table[y][x].value;
        if (value & 0x80000000) continue;
        uint32_t lum = ((value - date*(1<<28)) >> 22) & 0x1ff;
        if (lum >= 0x100) { lum = 0x1ff - lum; }
        Point<float> g;
        if (not grad( _table, Point<int32_t>(x,y), g )) continue;
        double sqn = g.sqnorm();
        int32_t decay = 128*std::max(1.-sqn, 0.);
        uint32_t alpha = (lum*decay >> 8) & 0xff;
        uint32_t a = 256-alpha, b = (alpha+1)*0xff;
        screen[y][x].b = (a*screen[y][x].b + b) >> 8;
        screen[y][x].g = (a*screen[y][x].g + b) >> 8;
        screen[y][x].r = (a*screen[y][x].r + b) >> 8;
      }
    }
  }

  template <typename mapT, typename actionT>
  Point<float>
  motion( mapT const& _table, actionT& _action )
  {
    Point<float> g;
    Point<float> const& pos =_action.pos();
    if (not grad( _table, pos.rebind<int32_t>(), g )) return Point<float>();
    double module = ::sqrt( g.sqnorm() );
    if (module < 1e-6) return Point<float>();
    return ((g / module) / (0.025 + module)).rebind<float>();
  }
  
  struct Delay
  {
    int32_t value;
    void set( double _value )
    {
      if (isnan( _value )) { value = 0x80000000; return; }
      double rem = fmod( _value, 8. );
      if (rem < 0) rem += 8.;
      value = int32_t( rem*double(1<<28) );
    }
  };
};

#endif /* __HYDRO_HH__ */
