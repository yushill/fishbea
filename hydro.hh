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
    if ((pos.m_y<=0) or (pos.m_y>=(VideoConfig::height-1)) or (pos.m_x<=0) or (pos.m_x>=(VideoConfig::width-1))) return false;
    int32_t vx0; if ((vx0 = _table[pos.m_y][pos.m_x-1]) == 0x80000000) return false;
    int32_t vx1; if ((vx1 = _table[pos.m_y][pos.m_x+1]) == 0x80000000) return false;
    int32_t vy0; if ((vy0 = _table[pos.m_y-1][pos.m_x]) == 0x80000000) return false;
    int32_t vy1; if ((vy1 = _table[pos.m_y+1][pos.m_x]) == 0x80000000) return false;
    _grad.m_x = double( vx1-vx0 ) / 2 / double( 1<<16 );
    _grad.m_y = double( vy1-vy0 ) / 2 / double( 1<<16 );
    return true;
  }


  template <typename mapT, typename actionT>
  void effect( mapT const& _table, actionT& _action )
  {
    SDL_Surface* scratch = _action.scratch();
    date_t date = _action.now();
    uint8_t* img = (uint8_t*)scratch->pixels;
    for (int y = 0, ystop = scratch->h; y < ystop; ++y) {
      uint8_t* line = &img[y*scratch->w*4];
      for (int x = 0, xstop = scratch->w; x < xstop; ++x) {
        uint8_t* pix = &line[x*4];
        pix[0] = 0xff;pix[1] = 0xff;pix[2] = 0xff;pix[3] = 0xff;
        uint8_t* alpha = &pix[3];
        int32_t value = _table[y][x];
        if (value == int32_t(0x80000000)) { *alpha = 0; continue; }
        uint32_t lum = ((value - date*(1<<16)) >> 10) & 0x1ff;
        if (lum >= 0x100) { lum = 0x1ff - lum; }
        Point<float> g;
        if (not grad( _table, Point<int32_t>(x,y), g )) { *alpha = 0; continue; }
        double sqn = g.sqnorm();
        int32_t decay = 256*std::max(1.-sqn, 0.) ;
        // int32_t decay = 256-value*8/(1<<16);
        *alpha = lum*decay >> 8;
      }
    }

    _action.blit( _action.scratch( scratch ) );
  }

  template <typename mapT>
  Point<float>
  motion( mapT const& _table, Point<float> const& pos, uintptr_t date )
  {
    Point<float> g;
    if (not grad( _table, pos.rebind<int32_t>(), g )) return Point<float>();
    double module = ::sqrt( g.sqnorm() );
    if (module < 1e-6) return Point<float>();
    return ((g / module) / (0.025 + module)).rebind<float>();
  }

};

#endif /* __HYDRO_HH__ */
