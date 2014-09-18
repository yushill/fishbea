#include <epmap.hh>
#include <action.hh>
#include <video.hh>
#include <hydro.hh>
#include <iostream>
#include <sstream>
#include <gallery.hh>
#include <cmath>

namespace {
  struct EpHydroField
  {
    hydro::Delay table[Screen::height][Screen::width];
    EpHydroField()
    {
      for (uintptr_t y = 0; y < Screen::height; ++y) {
        for (uintptr_t x = 0; x < Screen::width; ++x) {
          Point<float> pos( 479.5-x, 191.5-y );
          double sqnorm = pos.sqnorm();
          table[y][x].set( sqnorm < 160*160 ? (sqnorm*sqrt( sqnorm )/65536) : nan("") );
        }
      }
    }
  } ephf;

  struct EPRoomBuf : public virtual RoomBuf
  {
    int cmp( RoomBuf const& _rb ) const { return 0; }
    std::string getname() const { std::ostringstream oss; oss << "EPRoom[" << std::hex << TheCode << "]"; return oss.str(); }
  
    struct Code
    {
      Room room;
      uint32_t value;
      Code( Room _room ) : room(_room), value(0) {}
      bool match( Room _room, Point<int32_t> const& _pos, bool fire )
      {
        if (_room != room or not fire) return false;
        int idx = (_pos.y >> 6) - 1;
        if ((idx < 0) or (idx >= 4)) return false;
        if ((Point<int32_t>( _pos.x, _pos.y & 63 ) - Point<int32_t>(64,32)).sqnorm() > 24*24) return false;
        value |= (1 << idx);
        return false;
      }
      bool bit( int idx ) { return (value >> idx) & 1; }
    };
  
    void
    process( Action& _action ) const
    {
      // Collision items precomputed from scene rendering
      Code code( this );
      {
        TimeLine *tl = _action.m_story.active, *eotl = _action.m_story.active;
        do { tl->match( _action.m_story.now(), code ); } while ((tl = tl->fwd()) != eotl);
      }

      // Scene Draw
      _action.cornerblit( Point<int32_t>(), gallery::classic_bg );
      for (int door = 0; door < 4; ++door )
        _action.centerblit( Point<int32_t>( 64, 96+64*door ), code.bit( door ) ? gallery::shiny_starfish : gallery::starfish );
  
      static Point<float> const exitpos( 479.5, 191.5 );
      bool fishexit = (_action.pos() - exitpos).sqnorm() <= 24*24;
  
      _action.normalmotion();
    
      if (code.value == TheCode) {
        // draw exit
        if (_action.fires() and fishexit)
          {
            _action.moveto( EPMap::end_upcoming() );
            std::cerr << "Entering room: " << _action.m_room->getname() << ".\n";
            _action.fired();
            return;
          }
      } else {
        hydro::effect( ephf.table, _action );
        _action.moremotion( hydro::motion( ephf.table, _action ) );
      }
      _action.centerblit( exitpos.rebind<int32_t>(), fishexit ? gallery::shiny_shell : gallery::shell );
    }

    static uint32_t const TheCode = 0xa;
  };
}

Gate EPMap::start_incoming() { return Gate( new EPRoomBuf, Point<int32_t>(320, 192) ); }
Gate EPMap::end_incoming() { return Gate( new EPRoomBuf, Point<int32_t>(480, 192) ); }

