#include <pebblemap.hh>
#include <action.hh>
#include <video.hh>
#include <hydro.hh>
#include <SDL/SDL.h>
#include <iostream>
#include <sstream>
#include <gallery.hh>
#include <cmath>

namespace {
  struct DiaMeshRoomBuf : public virtual RoomBuf
  {
    int cmp( RoomBuf const& _rb ) const { return 0; }
    std::string getname() const { std::ostringstream oss; oss << "DiaMeshRoom"; return oss.str(); }
    
    struct Pebbling
    {
      Room room;
      uint32_t values;
      static int const side = 5;
      static int32_t const blocsize = 64;
      
      Pebbling( Room _room ) : room(_room), values() {}
      bool get( Point<int32_t> const& pos ) const { return (values >> (pos.m_y*side+pos.m_x)) & 1; }
      bool match( Room _room, Point<int32_t> pos, bool fire )
      {
        if (_room != room) return false;
        pos -= (Point<int32_t>( VideoConfig::width, VideoConfig::height ) - Point<int32_t>(side,side) * blocsize) / 2;
        if ((pos.m_y < 0) or (pos.m_x < 0)) return false;
        pos /= blocsize;
        if ((pos.m_y >= side) or (pos.m_x >= side)) return false;
        values |= (1 << (pos.m_y*side+pos.m_x));
        return false;
      }
      void draw( Action& _action ) const
      {
        for (int idx = 0; idx < side*side; ++ idx) {
          Point<int32_t> pos( 2*(idx % side)+1-side, 2*(idx/side)+1-side );
          pos = (pos*blocsize + Point<int32_t>( VideoConfig::width, VideoConfig::height ))/2;
          Pixel bloc[blocsize][blocsize];
          uint8_t alpha = ((values >> idx) & 1) ? 170 : 85;
          for (int pix = 0; pix < blocsize*blocsize; ++pix)
            bloc[pix/blocsize][pix%blocsize].set( 0xff, 0xff, 0xff, alpha );
          _action.blit( pos, bloc );
        }
      }
    };
    
    void
    process( Action& _action ) const
    {
      // Collision items precomputed from scene rendering
      Pebbling pebbling( this );
      {
        TimeLine *tl = _action.m_story.active, *eotl = _action.m_story.active;
        do { tl->match( _action.m_story.now(), pebbling ); } while ((tl = tl->fwd()) != eotl);
      }

      // Scene Draw
      _action.blit( gallery::classic_bg );
      pebbling.draw( _action );
  
      _action.normalmotion();
    
    }
  };
}

Gate DiaMesh::start_incoming() { return Gate( new DiaMeshRoomBuf, Point<int32_t>(50, 192) ); }
// Gate DiaMesh::end_incoming() { return Gate( new DiaMeshRoomBuf, Point<int32_t>(480, 192) ); }

