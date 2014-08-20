#include <dmmap.hh>
#include <action.hh>
#include <iostream>
#include <sstream>
#include <gallery.hh>

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
};

namespace
{
  template <typename int_type>
  static Point<int_type> direction( int_type _index )
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
  
  struct DMDoor
  {
    DMRoomBuf const* m_room;
    int32_t          m_index;

    DMDoor() : m_room(), m_index(-1) {};
    DMDoor( DMRoomBuf const* _room, int32_t _index = -1 ) : m_room( _room ), m_index( _index ) {}
    ~DMDoor() {}
    
    bool next() { return (++m_index < 4); }
  
    Point<int32_t> getpos() const
    {
      int32_t seed = m_room->m_index;
      int index = m_index;
      {
        int perms[4] = {0,1,2,3};
        for (int perm = 0; perm <= index; ++perm) {
          int pos = perm + (seed % (4-perm));
          if (pos > perm) { int tmp = perms[perm]; perms[perm] = perms[pos]; perms[pos] = tmp; }
        }
        index = perms[index];
      }
      return Point<int32_t>( VideoConfig::width/2, VideoConfig::height/2 ) + direction( index )*96;
    }
    
    Gate destination() const
    {
      // compute destination room and position (with destination door)
      DMRoomBuf* r = new DMRoomBuf( m_room->m_index + offset( direction( m_index ), DMRoomBuf::MySide ) );
      return Gate( r, DMDoor( r, m_index ^ 3 ).getpos() );
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
  for (DMDoor door( this ); door.next(); )
    {
      Point<int32_t> pos = door.getpos();
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
      if ((m_index == (MySide*MySide-1)) and (active.m_index == 3)) _action.moveto( DMMap::end_upcoming() );
      else if ((m_index == 0) and (active.m_index == 0))            _action.moveto( DMMap::start_upcoming() );
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

Gate DMMap::start_incoming() { DMRoomBuf* r = new DMRoomBuf( 0 ); return Gate( r, DMDoor( r, 0 ).getpos() ); }
Gate DMMap::end_incoming() { DMRoomBuf* r = new DMRoomBuf( -1 ); return Gate( r, DMDoor( r, 3 ).getpos() ); }

