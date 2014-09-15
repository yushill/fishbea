#include <pebblemap.hh>
#include <action.hh>
#include <video.hh>
#include <hydro.hh>
#include <SDL/SDL.h>
#include <iostream>
#include <sstream>
#include <gallery.hh>
#include <cmath>
#include <cstdlib>

namespace {
  template <typename RoomBufT>
  void
  wallcolor( RoomBufT const& rb, int32_t w, int32_t n, int32_t now, Pixel& pix )
  {
    bool const mid = (std::abs(w - RoomBufT::blocsize/2) < 8);
    uint8_t const a = mid ? (16*now + 32*n) : 0;
    uint8_t const b = mid ? 0xff : 0;
    pix.set( a, a, b, 0xff );
  }
  
  enum PebbleWall { PLAIN=0, FWD, BWD, NONE };
  
  template <typename RoomBufT>
  struct Pebbling
  {
    Room room;
    uintptr_t count;
    typename RoomBufT::PebbleBoard board;
    
    Pebbling( RoomBufT const& _roombuf ) : room(&_roombuf), count(0), board(_roombuf) {}
    bool match( int when, Room _room, Point<int32_t> pos, bool fire )
    {
      if ((_room != room) or (when > 0)) return false;
      board.append();
      pos -= (VideoConfig::diag() - Point<int32_t>(RoomBufT::side,RoomBufT::side) * RoomBufT::blocsize) / 2;
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
    // Vertical walls
    _action.normalmotion();
    
    // Collision items precomputed from scene rendering
    Pebbling<RoomBufT> pebbling( roombuf );
    {
      TimeLine *tl = _action.m_story.active, *eotl = _action.m_story.active;
      do { tl->find( _action.m_story.now(), pebbling ); } while ((tl = tl->fwd()) != eotl);
    }
    
    {
      Pixel bloc[roombuf.blocsize][roombuf.blocsize];
      for (int pix = 0; pix < roombuf.blocsize*roombuf.blocsize; ++pix)
        bloc[pix/roombuf.blocsize][pix%roombuf.blocsize].set( 0xff, 0xff, 0xff, 0xff );
      
      for (int32_t y = 0; y < roombuf.side; ++y) {
        for (int32_t x = 0; x < roombuf.side; ++x) {
          Point<int32_t> blocpos( 2*x+1-roombuf.side, 2*y+1-roombuf.side );
          blocpos = (blocpos*roombuf.blocsize + VideoConfig::diag())/2;
          uint8_t alpha = pebbling.board.active( mkpoint(x,y) ) ? 170 : 85;
          for (int pix = 0; pix < roombuf.blocsize*roombuf.blocsize; ++pix)
            bloc[pix/roombuf.blocsize][pix%roombuf.blocsize].a = alpha;
          _action.blit( blocpos, bloc );
        }
      }
    }
    
    { // Horizontal walls
      Pixel bloc[3][6][roombuf.blocsize];
      for (int32_t y = 0; y < 6; ++y) {
        for (int32_t x = 0; x < roombuf.blocsize; ++x) {
          bloc[0][y][x].set( 0, 0, 0, 0xff );
          wallcolor( roombuf, x, -y, _action.now(), bloc[1][y][x] );
          wallcolor( roombuf, x, +y, _action.now(), bloc[2][y][x] );
        }
      }
      
      for (int32_t y = 0; y < (roombuf.side+1); ++y) {
        for (int32_t x = 0; x < (roombuf.side+0); ++x) {
          PebbleWall wall = pebbling.board.hwall( x, y );
          if (wall == NONE) continue;
          Point<int32_t>
            pt1( (Point<int32_t>( 2*x-roombuf.side, 2*y-roombuf.side )*roombuf.blocsize + VideoConfig::diag())/2 ),
            pt2( (pt1 + Point<int32_t>( roombuf.blocsize, 0 )) );
          _action.blit( (pt1+pt2)/2, bloc[wall] );
          _action.cutmotion( pt1.rebind<float>(), pt2.rebind<float>() );
        }
      }
    }
    
    { // Vertical walls
      Pixel bloc[3][roombuf.blocsize][6];
      for (int32_t y = 0; y < roombuf.blocsize; ++y)
        for (int32_t x = 0; x < 6; ++x)
          {
            bloc[0][y][x].set( 0, 0, 0, 0xff );
            wallcolor( roombuf, y, -x, _action.now(), bloc[1][y][x] );
            wallcolor( roombuf, y, +x, _action.now(), bloc[2][y][x] );
          }
      
      for (int32_t y = 0; y < (roombuf.side+0); ++y) {
        for (int32_t x = 0; x < (roombuf.side+1); ++x) {
          PebbleWall wall = pebbling.board.vwall( x, y );
          if (wall == NONE) continue;
          Point<int32_t>
            pt1( (Point<int32_t>( 2*x-roombuf.side, 2*y-roombuf.side )*roombuf.blocsize + VideoConfig::diag())/2 ),
            pt2( (pt1 + Point<int32_t>( 0, roombuf.blocsize )) );
          _action.blit( (pt1+pt2)/2, bloc[wall] );
          _action.cutmotion( pt1.rebind<float>(), pt2.rebind<float>() );
        }
      }
    }
    
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
      PebbleBoard(DiaMeshRoomBuf const&) : count(4) { for (int idx = 0; idx < side*side; ++idx) (&table[0][0])[idx] = false; }
      bool table[side][side];
      intptr_t count;
      void activate( Point<int32_t> const& p ) { table[p.m_y][p.m_x] = true; }
      bool active( Point<int32_t> const& p ) { return table[p.m_y][p.m_x]; }
      PebbleWall hwall( int32_t x, int32_t y )
      {
        if (count < 0) return PLAIN;
        if ((y == 0) and (x == 0)) return NONE;
        if ((y == 0) or (y >= side)) return PLAIN;
        bool va = table[y-1][x];
        if (x == 0) return va ? NONE : FWD;
        return (va and table[y][x-1]) ? NONE : FWD;
      }
      PebbleWall vwall( int32_t x, int32_t y )
      {
        if (count < 0) return PLAIN;
        if ((x == 0) and (y == 0)) return NONE;
        if ((x == 0) or (x >= side)) return PLAIN;
        bool ha = table[y][x-1];
        if (y == 0) return ha ? NONE : FWD;
        return (ha and table[y-1][x]) ? NONE : FWD;
      }
      void append() { count -= 1; }
    };
  };
}

Gate DiaMesh::start_incoming() { return Gate( new DiaMeshRoomBuf, Point<int32_t>(50, 190) ); }
// Gate DiaMesh::end_incoming() { return Gate( new DiaMeshRoomBuf, Point<int32_t>(480, 192) ); }

