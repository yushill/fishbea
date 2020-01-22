#ifndef __HYDRO_HH__
#define __HYDRO_HH__

#include <video.hh>
#include <timeline.hh>
#include <cmath>
#include <inttypes.h>

namespace hydro
{
  struct Delay
  {
    int32_t value;
    void set( double _value )
    {
      if (std::isnan( _value )) { value = 0x80000000; return; }
      double rem = fmod( _value, 8. );
      if (rem < 0) rem += 8.;
      value = int32_t( rem*double(1<<28) );
    }
  };
  
  template <typename mapT>
  bool grad( mapT const& _table, Point<int32_t> const& pos, Point<float>& _grad )
  {
    if ((pos.y<=0) or (pos.y>=(Screen::height-1)) or (pos.x<=0) or (pos.x>=(Screen::width-1))) return false;
    int32_t vx0; if ((vx0 = _table[pos.y][pos.x-1].value) & 0x80000000) return false;
    int32_t vx1; if ((vx1 = _table[pos.y][pos.x+1].value) & 0x80000000) return false;
    int32_t vy0; if ((vy0 = _table[pos.y-1][pos.x].value) & 0x80000000) return false;
    int32_t vy1; if ((vy1 = _table[pos.y+1][pos.x].value) & 0x80000000) return false;
    _grad.x = double( (vx1-vx0) << 1 ) / 2 / double( 1<<29 );
    _grad.y = double( (vy1-vy0) << 1 ) / 2 / double( 1<<29 );
    return true;
  }

  template <typename mapT, typename actionT>
  void effect( mapT const& _table, actionT& _action )
  {
    date_t date = _action.now();
    for (int y = 0, ystop = Screen::height; y < ystop; ++y) {
      for (int x = 0, xstop = Screen::width; x < xstop; ++x) {
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
        
        Pixel& pix = _action.thescreen.pixels[y][x];
        pix.b = (a*pix.b + b) >> 8;
        pix.g = (a*pix.g + b) >> 8;
        pix.r = (a*pix.r + b) >> 8;
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
};

#endif /* __HYDRO_HH__ */
