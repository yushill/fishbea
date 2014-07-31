#include <epmap.hh>
#include <action.hh>
#include <video.hh>
#include <hydro.hh>
#include <SDL/SDL.h>
#include <iostream>
#include <sstream>
#include <gallery.hh>
#include <cmath>

namespace {
  struct EpHydroMap
  {
    int32_t table[VideoConfig::height][VideoConfig::width];
    EpHydroMap()
    {
      for (uintptr_t y = 0; y < VideoConfig::height; ++y) {
        for (uintptr_t x = 0; x < VideoConfig::width; ++x) {
          Point<float> pos( 479.5-x, 191.5-y );
          double sqnorm = pos.sqnorm();
          table[y][x] = sqnorm < 160*160 ? int32_t((sqnorm*sqrt( sqnorm )/65536)*(1<<16)) : int32_t( 0x80000000 );
        }
      }
    }
  } ehm;

  struct Code
  {
    Room room;
    uint32_t value;
    Code( Room _room ) : room(_room), value(0) {}
    bool match( Room _room, Point<int32_t> const& _pos, bool fire )
    {
      if (_room != room or not fire) return false;
      int idx = (_pos.m_y >> 6) - 1;
      if (idx < 0 or idx >= 4) return false;
      if ((Point<int32_t>( _pos.m_x, _pos.m_y & 63 ) - Point<int32_t>(64,32)).sqnorm() > 24*24) return false;
      value |= (1 << idx);
      return false;
    }
    bool bit( int idx ) { return (value >> idx) & 1; }
  };
};

void
EPRoomBuf::process( Action& _action ) const
{
  // Collision items precomputed from scene rendering
  Code code( this );
  {
    TimeLine *tl = _action.m_story.active, *eotl = _action.m_story.active;
    do { tl->match( _action.m_story.now(), code ); } while ((tl = tl->fwd()) != eotl);
  }  

  // Scene Draw
  _action.blit( gallery::classic_bg );
  for (int door = 0; door < 4; ++door )
    _action.blit( Point<int32_t>( 64, 96+64*door ), code.bit( door ) ? gallery::shiny_starfish : gallery::starfish );
  
  static Point<float> const exitpos( 479.5, 191.5 );
  bool fishexit = (_action.m_pos - exitpos).sqnorm() <= 24*24;
  
  if (code.value == m_code) {
    // draw exit
    if (_action.fires() and fishexit)
      {
        _action.moveto( this->end_upcoming() );
        std::cerr << "Entering room: " << _action.m_room->getname() << ".\n";
        _action.fired();
        return;
      }
    else
      _action.normalmotion();
  } else {
    hydro::effect( ehm.table, _action );
    _action.biasedmotion( 16, hydro::motion( ehm.table, _action ) );
  }
  _action.blit( exitpos.rebind<int32_t>(), fishexit ? gallery::shiny_starfish : gallery::starfish );
}

Gate
EPRoomBuf::start_incoming()
{
  return Gate( new EPRoomBuf( 0xa ), Point<int32_t>(320, 192) );
}

std::string
EPRoomBuf::getname() const
{
  std::ostringstream oss;
  oss << "EPRoom[" << std::hex << m_code << "]";
  return oss.str();
}


