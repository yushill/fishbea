#include <pebblemap.hh>
#include <action.hh>
#include <video.hh>
#include <hydro.hh>
#include <iostream>
#include <sstream>
#include <gallery.hh>
#include <cmath>
#include <cstdlib>

namespace pebble {
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
      bool match( int when, Room _room, Point<int32_t> const& _pos, bool fire )
      {
        if ((_room != room) or (when > 0)) return false;
        board.append();
        Point<int32_t> pos;
        if (not gridpos( _pos, pos )) return false;
        board.activate( pos );
        return false;
      }
    
      static bool gridpos( Point<int32_t> const& _initial, Point<int32_t>& _final )
      {
        Point<int32_t> tmp( _initial );
        tmp -= ((Screen::diag() - Point<int32_t>(RoomBufT::side,RoomBufT::side) * RoomBufT::blocsize) / 2);
        if ((tmp.y < 0) or (tmp.x < 0)) return false;
        tmp /= RoomBufT::blocsize;
        if ((tmp.y >= RoomBufT::side) or (tmp.x >= RoomBufT::side)) return false;
        _final = tmp;
        return true;
      }
    };
  
    template <typename RoomBufT>
    void pebbleprocess( RoomBufT const& roombuf, Action& _action )
    {
      // Vertical walls
      _action.normalmotion();
    
      // Collision items precomputed from scene rendering
      Pebbling<RoomBufT> pebbling( roombuf );
      {
        TimeLine *tl = _action.story().active, *eotl = _action.story().active;
        do { tl->find( _action.story().now(), pebbling ); } while ((tl = tl->fwd()) != eotl);
      }
    
      {
        for (int32_t y = 0; y < roombuf.side; ++y) {
          for (int32_t x = 0; x < roombuf.side; ++x) {
            Point<int32_t> blocpos( 2*x+1-roombuf.side, 2*y+1-roombuf.side );
            blocpos = (blocpos*roombuf.blocsize + Screen::diag())/2;
            uint8_t alpha = (pebbling.board.active( mkpoint(x,y) ) ? 170 : 85);
            uint16_t b = (alpha+1)*0xff, a = (256-alpha);
            for (int32_t yb = -(roombuf.blocsize/2); yb < (roombuf.blocsize/2); ++yb) {
              for (int32_t xb = -(roombuf.blocsize/2); xb < (roombuf.blocsize/2); ++xb) {
                Pixel& pix = _action.thescreen.pixels[blocpos.y+yb][blocpos.x+xb];
                pix.b = (a*pix.b + b) >> 8;
                pix.g = (a*pix.g + b) >> 8;
                pix.r = (a*pix.r + b) >> 8;
              }
            }
          }
        }
      }
    
      { // Horizontal walls
        Pixel bloc[3][6][roombuf.blocsize];
        for (int32_t y = 0; y < 6; ++y) {
          for (int32_t x = 0; x < roombuf.blocsize; ++x) {
            bloc[0][y][x].set( 0, 0, 0, 0xff );
            wallcolor( roombuf, x, y, -_action.now(), bloc[1][y][x] );
            wallcolor( roombuf, x, y, +_action.now(), bloc[2][y][x] );
          }
        }
      
        for (int32_t y = 0; y < (roombuf.side+1); ++y) {
          for (int32_t x = 0; x < (roombuf.side+0); ++x) {
            PebbleWall wall = pebbling.board.hwall( x, y );
            if (wall == NONE) continue;
            Point<int32_t>
              pt1( (Point<int32_t>( 2*x-roombuf.side, 2*y-roombuf.side )*roombuf.blocsize + Screen::diag())/2 ),
              pt2( (pt1 + Point<int32_t>( roombuf.blocsize, 0 )) );
            _action.centerblit( (pt1+pt2)/2, bloc[wall] );
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
              wallcolor( roombuf, y, x, -_action.now(), bloc[1][y][x] );
              wallcolor( roombuf, y, x, +_action.now(), bloc[2][y][x] );
            }
      
        for (int32_t y = 0; y < (roombuf.side+0); ++y) {
          for (int32_t x = 0; x < (roombuf.side+1); ++x) {
            PebbleWall wall = pebbling.board.vwall( x, y );
            if (wall == NONE) continue;
            Point<int32_t>
              pt1( (Point<int32_t>( 2*x-roombuf.side, 2*y-roombuf.side )*roombuf.blocsize + Screen::diag())/2 ),
              pt2( (pt1 + Point<int32_t>( 0, roombuf.blocsize )) );
            _action.centerblit( (pt1+pt2)/2, bloc[wall] );
            _action.cutmotion( pt1.rebind<float>(), pt2.rebind<float>() );
          }
        }
      }
    
      if (_action.cmds[Action::Fire]) {
        Point<int32_t> pos;
        if (pebbling.gridpos( _action.pos().rebind<int32_t>(), pos )) {
          _action.cmds.reset();
          roombuf.moveto( _action , pos );
        }
      }
    }
  
    struct DiaMeshRoomBuf : public virtual RoomBuf
    {
      int cmp( RoomBuf const& _rb ) const { return 0; }
      std::string getname() const { std::ostringstream oss; oss << "DiaMeshRoom"; return oss.str(); }
    
      static int const side = 5;
      static int32_t const blocsize = 64;
    
      void moveto( Action& _action, Point<int32_t> const& _pos ) const
      {
        if ((_pos.x == (side-1)) and (_pos.y == (side-1)))
          _action.moveto( DiaMesh::end_upcoming() );
        else
          _action.moveto( DiaMesh::start_incoming() );
      }
    
      struct PebbleBoard
      {
        PebbleBoard(DiaMeshRoomBuf const&) : count(5) { for (int idx = 0; idx < side*side; ++idx) (&table[0][0])[idx] = false; }
        bool table[side][side];
        intptr_t count;
        void activate( Point<int32_t> const& p ) { table[p.y][p.x] = true; }
        bool active( Point<int32_t> const& p ) { return table[p.y][p.x]; }
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
    
      void
      process( Action& _action ) const
      {
        _action.cornerblit( Point<int32_t>(), gallery::classic_bg );
        pebbleprocess( *this, _action );
      }
    };
  
    struct SimpleRoomBuf : public virtual RoomBuf
    {
      int cmp( RoomBuf const& _rb ) const { return 0; }
      std::string getname() const { std::ostringstream oss; oss << "SimpleRoom"; return oss.str(); }
    
      static int const side = 5;
      static int32_t const blocsize = 64;
    
      void moveto( Action& _action, Point<int32_t> const& _pos ) const
      {
        if ((_pos.x == side/2) and (_pos.y == side/2))
          _action.moveto( Simple::end_upcoming() );
        else
          _action.moveto( Simple::start_incoming() );
      }
    
      struct PebbleBoard
      {
        PebbleBoard(SimpleRoomBuf const&) : count(2) { for (int idx = 0; idx < 10; ++idx) cells[idx] = false; }
        bool cells[10];
        intptr_t count;
        int cellidx( int32_t x, int32_t y )
        {
          int const map [side*side] = {
            0,1,1,1,1,
            3,4,5,5,1,
            2,7,8,5,1,
            1,6,5,5,1,
            1,1,1,1,1,
          };
          if ((uint32_t( y ) >= side) or (uint32_t( x ) >= side)) return 9;
          return map[y*side+x];
        }
        void activate( Point<int32_t> const& p ) { cells[cellidx(p.x,p.y)] = true; }
        bool active( Point<int32_t> const& p ) { return cells[cellidx(p.x,p.y)]; }
        PebbleWall hwall( int32_t x, int32_t y ) { return wall( cellidx( x, y-1 ), cellidx( x, y ) ); }
        PebbleWall vwall( int32_t x, int32_t y ) { return wall( cellidx( x-1, y ), cellidx( x, y ) ); }
        PebbleWall rwall( int src, int dst ) { PebbleWall rev = wall( dst, src ); return rev == FWD ? BWD : rev; }
        PebbleWall wall( int src, int dst )
        {
          if (src == dst) return NONE;
          if (count < 0) return PLAIN;
          switch (dst) {
          case 0: switch (src) { case 9: return NONE; case 1: case 3: return rwall( src, dst ); } break;
          case 1: switch (src) { case 0: return (cells[0] ? NONE : FWD); case 2: return rwall( src, dst ); } break;
          case 2: switch (src) { case 1: return (cells[1] ? NONE : FWD); case 3: return rwall( src, dst ); } break;
          case 3: switch (src) { case 0: case 2: return ((cells[0] and cells[2]) ? NONE : FWD); case 4: return rwall( src, dst ); } break;
          case 4: switch (src) { case 3: return (cells[3] ? NONE : FWD); case 5: case 7: return rwall( src, dst ); } break;
          case 5: switch (src) { case 4: return (cells[4] ? NONE : FWD); case 6: return rwall( src, dst ); } break;
          case 6: switch (src) { case 5: return (cells[5] ? NONE : FWD); case 7: return rwall( src, dst ); } break;
          case 7: switch (src) { case 4: case 6: return ((cells[4] and cells[6]) ? NONE : FWD); case 8: return rwall( src, dst ); } break;
          case 8: switch (src) { case 7: return (cells[7] ? NONE : FWD); } break;
          case 9: switch (src) { case 0: return NONE; } break;
          }
          return PLAIN;
        }
        void append() { count -= 1; }
      };

      void
      process( Action& _action ) const
      {
        // if ((pos.x == (roombuf.side-1)) and (pos.y == (roombuf.side-1)))
        //   _action.moveto( roombuf.end_upcoming() );
        // else
        //   _action.moveto( roombuf.start_incoming() );
  
        _action.cornerblit( Point<int32_t>(), gallery::classic_bg );
        pebbleprocess( *this, _action );
      }
    };
  }

  Gate DiaMesh::start_incoming() { return Gate( new DiaMeshRoomBuf, Point<int32_t>(50, 190) ); }

  Gate Simple::start_incoming() { return Gate( new SimpleRoomBuf, Point<int32_t>(50, 190) ); }

  Gate Simple::end_upcoming() { return DiaMesh::start_incoming(); }
};
