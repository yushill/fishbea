#include <slideroom.hh>
#include <action.hh>
#include <video.hh>
#include <hydro.hh>
#include <iostream>
#include <sstream>
#include <gallery.hh>
#include <cmath>

namespace
{
  struct SpiralHydroField
  {
    hydro::Delay table[Screen::height][Screen::width];
    SpiralHydroField()
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
          double normal = angle*16/M_PI;
          table[y][x].set( normal + radial );
        }
      }
    }
  } SpiralHF;
  
  struct SpiralRoomBuf : public virtual RoomBuf
  {
    int cmp( RoomBuf const& _rb ) const { return 0; }
    std::string getname() const { return "SpiralRoom"; }
  
    void
    process( Action& _action ) const
    {
      // Scene Draw
      _action.cornerblit( Point<int32_t>(), gallery::classic_bg );

      bool over_end, over_start;
  
      {
        Point<int32_t> pos( Screen::diag()/2 );
        over_end = (pos.rebind<float>() - _action.pos()).sqnorm() <= 24*24;
        _action.centerblit( pos, over_end ? gallery::shiny_shell : gallery::shell );
      }
      {
        Point<int32_t> pos( 50, 50 );
        over_start = (pos.rebind<float>() - _action.pos()).sqnorm() <= 24*24;
        _action.centerblit( pos, over_start ? gallery::shiny_shell : gallery::shell );
      }
  
      if (_action.fires()) {
        if (over_end) { _action.moveto( Spiral::end_upcoming() ); }
        if (over_start) { _action.moveto( Spiral::start_upcoming() ); }
        if (over_end or over_start) { _action.fired(); return; }
      }
  
      hydro::effect( SpiralHF.table, _action );
      _action.normalmotion();
      _action.moremotion( hydro::motion( SpiralHF.table, _action ) );
    }
  };
}

Gate Spiral::start_incoming() { return Gate( new SpiralRoomBuf, Point<int32_t>(50, 50) ); }
Gate Spiral::end_incoming() { return Gate( new SpiralRoomBuf, Point<int32_t>(Screen::width/2, Screen::height/2) ); }

namespace {
  struct SlalomHydroField
  {
    hydro::Delay table[Screen::height][Screen::width];
    SlalomHydroField()
    {
      for (int32_t y = 0; y < Screen::height; ++y) {
        for (int32_t x = 0; x < Screen::width; ++x) {
          if ((x < 40) or (x > (Screen::width-40))) {
            table[y][x].set( nan("") );
          } else if ((y < 40) or (y > (Screen::height-40))) {
            table[y][x].set( (40-x)/27. );
          } else {
            float center = (sin( (x+40)*M_PI*2./160. )*128 + Screen::height/2);
            table[y][x].set( 7 * std::max( ((center - y) / float(center - 32)), ((center - y) / float(center - (Screen::height-32))) ) );
          }
        }
      }
    }
  } SlalomHF;
  
  struct SlalomRoomBuf : public virtual RoomBuf
  {
    int cmp( RoomBuf const& _rb ) const { return 0; }
    std::string getname() const { return "SlalomRoom"; }
    
    void wall( Action& _action, int32_t x, int32_t y1, int32_t y2 ) const
    {
      if (y1 > y2) std::swap( y1, y2 );
      static int32_t const radius = 2;
      Point<int32_t> beg( x, y1 ), end( x, y2 );
      _action.cutmotion( beg.rebind<float>(), end.rebind<float>() );
      beg -= Point<int32_t>(radius,radius);
      end += Point<int32_t>(radius,radius);
      int32_t ybeg, yend, xbeg, xend;
      beg.pull( xbeg, xend );
      end.pull( ybeg, yend );
      for (int32_t x = xbeg; x < xend; ++x) {
        for (int32_t y = ybeg; y < yend; ++y) {
          _action.thescreen.pixels[y][x].set( 0, 0, 0, 0xff );
        }
      }
    }
    
    void
    process( Action& _action ) const
    {
      // Scene Draw
      _action.cornerblit( Point<int32_t>(), gallery::classic_bg );
    
      // Hydro field
      hydro::effect( SlalomHF.table, _action );
      _action.normalmotion();
      _action.moremotion( hydro::motion( SlalomHF.table, _action ) );
    
      // Walls
      for (int idx = 0; idx < 4; ++idx)
        {
          wall( _action,  40 + idx*160, 40, Screen::height/2-24 );
          wall( _action,  40 + idx*160, Screen::height/2, Screen::height-40 );
          wall( _action, 120 + idx*160, 40, Screen::height/2 );
          wall( _action, 120 + idx*160, Screen::height/2+24, Screen::height-40 );
        }
      {
        static Point<float> m1;
        Point<float> m2( _action.pos() );
        float px = fmod( m2.x, 80 );
        std::swap( px, m2.x );
        if ((m1.x <= 40.) and (m2.x >= 40.)) {
          Point<float> md = m2 - m1;
          Point<float> pos( px - m2.x + 40, m1.y + md.y*(40 - m1.x)/md.x );
          std::cout << "position: {" << pos.x << ',' << pos.y << "}\n";
        }
        m1 = m2;
      }
      _action.cutmotion( Point<float>( 0, 0 ), Point<float>( Screen::width, 0 ) );
      _action.cutmotion( Point<float>( Screen::width, 0 ), Point<float>( Screen::width, Screen::height ) );
      _action.cutmotion( Point<float>( Screen::width, Screen::height ), Point<float>( 0, Screen::height ) );
      _action.cutmotion( Point<float>( 0, Screen::height ), Point<float>( 0, 0 ) );
    }
  };
}

Gate Slalom::start_incoming() { return Gate( new SlalomRoomBuf, Point<int32_t>(20, 20) ); }
Gate Slalom::end_incoming() { return Gate( new SlalomRoomBuf, Point<int32_t>(Screen::width/2, Screen::height/2) ); }


