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
  enum PebbleWall { PLAIN=0, FWD, BWD, NONE };
  
  template <typename RoomBufT>
  struct Pebbling
  {
    Room room;
    typename RoomBufT::PebbleBoard board;
    
    Pebbling( RoomBufT const& _roombuf ) : room(&_roombuf), board(_roombuf) {}
    bool match( int when, Room _room, Point<int32_t> pos, bool fire )
    {
      if ((_room != room) or (when > 0)) return false;
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
      Pixel bloc[3][4][roombuf.blocsize];
      for (int32_t y = 0; y < 4; ++y)
        for (int32_t x = 0; x < roombuf.blocsize; ++x)
          {
            bloc[0][y][x].set( 0, 0, 0, 0xff );
            bloc[1][y][x].set( 85*~y, 85*~y, 85*~y, 0xff );
            bloc[2][y][x].set( 85*y, 85*y, 85*y, 0xff );
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
      Pixel bloc[3][roombuf.blocsize][4];
      for (int32_t y = 0; y < roombuf.blocsize; ++y)
        for (int32_t x = 0; x < 4; ++x) {
          bloc[0][y][x].set( 0, 0, 0, 0xff );
          bloc[1][y][x].set( 85*~x, 85*~x, 85*~x, 0xff );
          bloc[2][y][x].set( 85*x, 85*x, 85*x, 0xff );
        }
      
      for (int32_t y = 0; y < (roombuf.side+0); ++y) {
        for (int32_t x = 0; x < (roombuf.side+1); ++x) {
          PebbleWall wall = pebbling.board.vwall( x, y );
          if (wall == NONE) continue;
          Point<int32_t>
            pt1( (Point<int32_t>( 2*x-roombuf.side, 2*y-roombuf.side )*roombuf.blocsize + VideoConfig::diag())/2 ),
            pt2( (pt1 + Point<int32_t>( 0, roombuf.blocsize )) );
          _action.blit( (pt1+pt2)/2, bloc[0] );
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
      PebbleBoard(DiaMeshRoomBuf const&) { for (int idx = 0; idx < side*side; ++idx) (&table[0][0])[idx] = false; }
      bool table[side][side];
      void activate( Point<int32_t> const& p ) { table[p.m_y][p.m_x] = true; }
      bool active( Point<int32_t> const& p ) { return table[p.m_y][p.m_x]; }
      PebbleWall hwall( int32_t x, int32_t y )
      {
        if ((y == 0) and (x == 0)) return NONE;
        if ((y == 0) or (y >= side)) return PLAIN;
        bool va = table[y-1][x];
        if (x == 0) return va ? NONE : FWD;
        return (va and table[y][x-1]) ? NONE : FWD;
      }
      PebbleWall vwall( int32_t x, int32_t y )
      {
        if ((x == 0) and (y == 0)) return NONE;
        if ((x == 0) or (x >= side)) return PLAIN;
        bool ha = table[y][x-1];
        if (y == 0) return ha ? NONE : FWD;
        return (ha and table[y-1][x]) ? NONE : FWD;
      }
    };
  };
}

Gate DiaMesh::start_incoming() { return Gate( new DiaMeshRoomBuf, Point<int32_t>(50, 192) ); }
// Gate DiaMesh::end_incoming() { return Gate( new DiaMeshRoomBuf, Point<int32_t>(480, 192) ); }

