#include <slideroom.hh>
#include <action.hh>
#include <video.hh>
#include <hydro.hh>
#include <SDL/SDL.h>
#include <iostream>
#include <sstream>
#include <gallery.hh>
#include <cmath>

namespace {
  struct SpiralHydroField
  {
    hydro::Delay table[VideoConfig::height][VideoConfig::width];
    SpiralHydroField()
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
          double normal = angle*16/M_PI;
          table[y][x].set( normal + radial );
        }
      }
    }
  } SpiralHF;
  
  struct SlalomHydroField
  {
    hydro::Delay table[VideoConfig::height][VideoConfig::width];
    SlalomHydroField()
    {
      for (uintptr_t y = 0; y < VideoConfig::height; ++y) {
        for (uintptr_t x = 0; x < VideoConfig::width; ++x) {
          if ((y < 32) or (y > (VideoConfig::height-32))) { table[y][x].set( nan("") ); continue; }
          if ((x < 32) or (x > (VideoConfig::width-32))) { table[y][x].set( nan("") ); continue; }
          float center = (sin( float( x ) * M_PI * 2 * 3 / VideoConfig::width ) * 128 + 192);
          table[y][x].set( 16 * std::max( ((center - y) / float(center - 32)), ((center - y) / float(center - (VideoConfig::height-32))) ) );
        }
      }
    }
  } SlalomHF;
  
};

struct SlideRoomBuf : public virtual RoomBuf
{
  int                   cmp( RoomBuf const& _rb ) const { return 0; }
  std::string           getname() const { return "SlideRoom"; }
  
  void
  process( Action& _action ) const
  {
    // Scene Draw
    _action.blit( gallery::classic_bg );

    bool over_end, over_start;
  
    {
      Point<int32_t> pos( VideoConfig::width/2, VideoConfig::height/2 );
      over_end = (pos.rebind<float>() - _action.pos()).sqnorm() <= 24*24;
      _action.blit( pos, over_end ? gallery::shiny_shell : gallery::shell );
    }
    {
      Point<int32_t> pos( 50, 50 );
      over_start = (pos.rebind<float>() - _action.pos()).sqnorm() <= 24*24;
      _action.blit( pos, over_start ? gallery::shiny_shell : gallery::shell );
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

Gate Spiral::start_incoming() { return Gate( new SlideRoomBuf, Point<int32_t>(50, 50) ); }
Gate Spiral::end_incoming() { return Gate( new SlideRoomBuf, Point<int32_t>(VideoConfig::width/2, VideoConfig::height/2) ); }

struct SlalomRoomBuf : public virtual RoomBuf
{
  int                   cmp( RoomBuf const& _rb ) const { return 0; }
  std::string           getname() const { return "SlalomRoom"; }
  
  void
  process( Action& _action ) const
  {
    // Scene Draw
    _action.blit( gallery::classic_bg );
    
    // Hydro field
    hydro::effect( SlalomHF.table, _action );
    _action.normalmotion();
    _action.moremotion( hydro::motion( SlalomHF.table, _action ) );
    
    // Walls
  }
};

Gate Slalom::start_incoming() { return Gate( new SlalomRoomBuf, Point<int32_t>(50, 50) ); }
Gate Slalom::end_incoming() { return Gate( new SlalomRoomBuf, Point<int32_t>(VideoConfig::width/2, VideoConfig::height/2) ); }

