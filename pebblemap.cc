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
  template <typename RoomBufT>
  struct Pebbling
  {
    Room room;
    typename RoomBufT::PebbleBoard board;
    
    Pebbling( RoomBufT const& _roombuf ) : room(&_roombuf), board(_roombuf) {}
    bool match( int when, Room _room, Point<int32_t> pos, bool fire )
    {
      if ((_room != room) or (when > 0)) return false;
      pos -= (Point<int32_t>( VideoConfig::width, VideoConfig::height ) - Point<int32_t>(RoomBufT::side,RoomBufT::side) * RoomBufT::blocsize) / 2;
      if ((pos.m_y < 0) or (pos.m_x < 0)) return false;
      pos /= RoomBufT::blocsize;
      if ((pos.m_y >= RoomBufT::side) or (pos.m_x >= RoomBufT::side)) return false;
      board.activate( pos );
      return false;
    }
  };
  
  template <typename RoomBufT>
  bool pebbleprocess( RoomBufT const& roombuf, Action& _action )
  {
    // Collision items precomputed from scene rendering
    Pebbling<RoomBufT> pebbling( roombuf );
    {
      TimeLine *tl = _action.m_story.active, *eotl = _action.m_story.active;
      do { tl->find( _action.m_story.now(), pebbling ); } while ((tl = tl->fwd()) != eotl);
    }

    for (int32_t y = 0; y < roombuf.side; ++y) {
      for (int32_t x = 0; x < roombuf.side; ++x) {
        Point<int32_t> blocpos( 2*x+1-roombuf.side, 2*y+1-roombuf.side );
        blocpos = (blocpos*roombuf.blocsize + Point<int32_t>( VideoConfig::width, VideoConfig::height ))/2;
        Pixel bloc[roombuf.blocsize][roombuf.blocsize];
        uint8_t alpha = pebbling.board.active( mkpoint(x,y) ) ? 170 : 85;
        for (int pix = 0; pix < roombuf.blocsize*roombuf.blocsize; ++pix)
          bloc[pix/roombuf.blocsize][pix%roombuf.blocsize].set( 0xff, 0xff, 0xff, alpha );
        _action.blit( blocpos, bloc );
      }
    }
    
    _action.normalmotion();
    return false;
  }
    
  struct DiaMeshRoomBuf : public virtual RoomBuf
  {
    int cmp( RoomBuf const& _rb ) const { return 0; }
    std::string getname() const { std::ostringstream oss; oss << "DiaMeshRoom"; return oss.str(); }
    
    static int const side = 5;
    static int32_t const blocsize = 64;
    
    void
    process( Action& _action ) const
    {
      _action.blit( gallery::classic_bg );
      bool victory = pebbleprocess( *this, _action );
      if (victory) _action.moveto( DiaMesh::end_upcoming() );
    }
    
    struct PebbleBoard
    {
      PebbleBoard(DiaMeshRoomBuf const&) { for (int idx = 0; idx < side*side; ++idx) (&table[0][0])[idx] = false; }
      bool table[side][side];
      void activate( Point<int32_t> const& p ) { table[p.m_y][p.m_x] = true; }
      bool active( Point<int32_t> const& p ) { return table[p.m_y][p.m_x]; }
    };
  };
}

Gate DiaMesh::start_incoming() { return Gate( new DiaMeshRoomBuf, Point<int32_t>(50, 192) ); }
// Gate DiaMesh::end_incoming() { return Gate( new DiaMeshRoomBuf, Point<int32_t>(480, 192) ); }

