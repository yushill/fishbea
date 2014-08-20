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
  struct SlideHydroMap
  {
    hydro::Delay table[VideoConfig::height][VideoConfig::width];
    SlideHydroMap()
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
          double normal = angle*8/M_PI;
          table[y][x].set( normal + radial );
        }
      }
    }
  } thm;
};

struct SlideRoomBuf : public virtual RoomBuf
{
  SlideRoomBuf() {}
  SlideRoomBuf( SlideRoomBuf const& _room ) { throw "NoNoNo"; }
  virtual ~SlideRoomBuf() {}
  
  std::string           getname() const { return "SlideRoom"; }
  void                  process( Action& _action ) const;
  int                   cmp( RoomBuf const& _rb ) const { return 0; }
};

void
SlideRoomBuf::process( Action& _action ) const
{
  // Scene Draw
  _action.blit( gallery::classic_bg );

  bool over_end, over_start;
  
  {
    Point<int32_t> pos( VideoConfig::width/2, VideoConfig::height/2 );
    over_end = (pos.rebind<float>() - _action.m_pos).sqnorm() <= 24*24;
    _action.blit( pos, over_end ? gallery::shiny_shell : gallery::shell );
  }
  {
    Point<int32_t> pos( 50, 50 );
    over_start = (pos.rebind<float>() - _action.m_pos).sqnorm() <= 24*24;
    _action.blit( pos, over_start ? gallery::shiny_shell : gallery::shell );
  }
  
  if (_action.fires()) {
    if (over_end) { _action.moveto( SlideMap::end_upcoming() ); }
    if (over_start) { _action.moveto( SlideMap::start_upcoming() ); }
    if (over_end or over_start) { _action.fired(); return; }
  }
  
  hydro::effect( thm.table, _action );
  _action.biasedmotion( hydro::motion( thm.table, _action ) );
}

Gate SlideMap::start_incoming() { return Gate( new SlideRoomBuf, Point<int32_t>(50, 50) ); }
Gate SlideMap::end_incoming() { return Gate( new SlideRoomBuf, Point<int32_t>(VideoConfig::width/2, VideoConfig::height/2) ); }
