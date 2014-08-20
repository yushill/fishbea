#include <dmmap.hh>
#include <action.hh>
#include <iostream>
#include <sstream>
#include <gallery.hh>

template <typename int_type>
Point<int_type> direction( int_type _index )
{
  // { 0: (-1, 0), 1: (0, -1), 2: (0, +1), 3: (+1, 0) }
  int_type a = _index & 1, b = (_index >> 1) & 1;
  return Point<int_type>( b + a - 1, b - a );
}
  
template <typename int_type>
int_type offset( Point<int_type> const& pt, int_type _width )
{
  return pt.m_y*_width+pt.m_x;
}

struct DMRoomBuf : public virtual RoomBuf
{
  static int32_t const  MySide = 4;
  int32_t               m_index;
  
  explicit DMRoomBuf( int32_t _index ) : m_index( (_index+MySide*MySide)%(MySide*MySide) ) {}
  DMRoomBuf( DMRoomBuf const& _room ) { throw "NoNoNo"; }
  virtual ~DMRoomBuf() {};
  
  int cmp( RoomBuf const& _rb ) const { return tgcmp( m_index, dynamic_cast<DMRoomBuf const&>( _rb ).m_index ); }
  
  std::string           getname() const;
  void                  process( Action& _action ) const;

  Point<int32_t>        door_position( int32_t door_index ) const
  {
    int32_t perm_code = m_index;
    int perm_table[4] = {0,1,2,3};
    for (int idx = 0; idx <= door_index; ++idx) {
      int pos = idx + (perm_code % (4-idx));
      perm_code /= (4-idx);
      if (pos > idx) { int tmp = perm_table[idx]; perm_table[idx] = perm_table[pos]; perm_table[pos] = tmp; }
    }
    door_index = perm_table[door_index];
    return Point<int32_t>( VideoConfig::width/2, VideoConfig::height/2 ) + direction( door_index )*96;
  }
    
};

namespace
{
  struct DMDoor
  {
    DMRoomBuf const* m_room;
    int32_t          door_index;

    DMDoor() : m_room(), door_index(-1) {};
    DMDoor( DMRoomBuf const* _room, int32_t _index = -1 ) : m_room( _room ), door_index( _index ) {}
    ~DMDoor() {}
    
    bool next() { return (++door_index < 4); }
  
    Gate destination() const
    {
      // compute destination room and position (with destination door)
      DMRoomBuf* r = new DMRoomBuf( m_room->m_index + offset( direction( door_index ), DMRoomBuf::MySide ) );
      return Gate( r, r->door_position( door_index ^ 3 ) );
    }
  };
};
  
void
DMRoomBuf::process( Action& _action ) const
{
  // Collision items precomputed from scene rendering
  DMDoor active;
    
  // std::cerr << "Subject pos: " << m_x << "," << m_y << ".\n";
  // Scene Draw
  _action.blit( gallery::classic_bg );
  for (int32_t index = 0; index < 4; ++index )
    {
      DMDoor door( this, index );
      Point<int32_t> pos = door_position( index );
      if ((pos.rebind<float>() - _action.m_pos).sqnorm() > 24*24) {
        _action.blit( pos, gallery::shell );
      } else {
        _action.blit( pos, gallery::shiny_shell );
        active = door;
      }
    }
  // GameWorld interaction
    
  if (active.m_room and _action.fires())
    {
      if ((m_index == (MySide*MySide-1)) and (active.door_index == 3)) _action.moveto( DMMap::end_upcoming() );
      else if ((m_index == 0) and (active.door_index == 0))            _action.moveto( DMMap::start_upcoming() );
      else                                                          _action.moveto( active.destination() );
      std::cerr << "Entering room: " << _action.m_room->getname() << ".\n";
      _action.fired();
    }
    
  else
    _action.normalmotion();
}

std::string
DMRoomBuf::getname() const
{
  std::ostringstream oss;
  oss << "DiamondRoom[" << m_index << "]";
  return oss.str();
}

Gate DMMap::start_incoming() { DMRoomBuf* r = new DMRoomBuf( 0 ); return Gate( r, r->door_position( 0 ) ); }
Gate DMMap::end_incoming() { DMRoomBuf* r = new DMRoomBuf( -1 ); return Gate( r, r->door_position( 3 ) ); }

