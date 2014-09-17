#include <dmmap.hh>
#include <action.hh>
#include <iostream>
#include <sstream>
#include <gallery.hh>

struct DMRoomBuf : public virtual RoomBuf
{
  explicit DMRoomBuf( int32_t _index ) : m_index( (_index+TheSide*TheSide)%(TheSide*TheSide) ) {}
  int cmp( RoomBuf const& _rb ) const { return tgcmp( m_index, dynamic_cast<DMRoomBuf const&>( _rb ).m_index ); }
  std::string getname() const { std::ostringstream oss; oss << "DiamondRoom[" << m_index << "]"; return oss.str(); }
  
  void
  process( Action& _action ) const
  {
    int32_t active_door = -1;
    
    // Scene Draw
    _action.cornerblit( Point<int32_t>(), gallery::classic_bg );
    for (int32_t door_idx = 0; door_idx < 4; ++door_idx )
      {
        Point<int32_t> pos = door_position( door_idx );
        if ((pos.rebind<float>() - _action.pos()).sqnorm() > 24*24) {
          _action.centerblit( pos, gallery::shell );
        } else {
          _action.centerblit( pos, gallery::shiny_shell );
          active_door = door_idx;
        }
      }
    
    // GameWorld interaction
    if ((active_door >= 0) and _action.fires())
      {
        if ((m_index == (TheSide*TheSide-1)) and (active_door == 3)) _action.moveto( DMMap::end_upcoming() );
        else if ((m_index == 0) and (active_door == 0))              _action.moveto( DMMap::start_upcoming() );
        else                                                         _action.moveto( door_upcoming( active_door ) );
        std::cerr << "Entering room: " << _action.m_room->getname() << ".\n";
        _action.fired();
      }
    
    else
      _action.normalmotion();
  }

  static int32_t const  TheSide = 4;
  static Point<int32_t> ThePositions( int32_t idx )
  { static int32_t const w[] = {1,0,-1,0}; return Point<int32_t>( w[idx&3], w[(idx+3)&3] ); }
  
  int32_t m_index;
  
  Point<int32_t>
  door_position( int32_t door_index ) const
  {
    // Shuffling door index according to room index, so that the
    // player can't easily find it's way.
    int32_t perm_code = m_index, perm_table[4] = {0,1,2,3};
    for (int32_t idx = 0; idx <= door_index; ++idx) {
      int32_t pos = idx + (perm_code % (4-idx)); perm_code /= (4-idx);
      if (pos > idx) std::swap( perm_table[idx], perm_table[pos] );
    }
    door_index = perm_table[door_index];
    // Returing door position according to shuffled index.
    return Point<int32_t>( VideoConfig::width/2, VideoConfig::height/2 ) + ThePositions( door_index )*96;
  }

  Gate
  door_upcoming( int32_t door_index ) const
  {
    Point<int32_t> pos = ThePositions( door_index );
    DMRoomBuf* r = new DMRoomBuf( m_index + pos.m_x + pos.m_y*TheSide );
    return Gate( r, r->door_position( door_index ^ 2 ) );
  }
};

Gate DMMap::start_incoming() { DMRoomBuf* r = new DMRoomBuf( 0 ); return Gate( r, r->door_position( 0 ) ); }
Gate DMMap::end_incoming() { DMRoomBuf* r = new DMRoomBuf( -1 ); return Gate( r, r->door_position( 3 ) ); }

