#ifndef __DMMAP_HH__
#define __DMMAP_HH__

#include <map.hh>

template <typename poinT>
bool nextdir( poinT& pt )
{
  int x = pt.m_x; pt.m_x = -pt.m_y; pt.m_y = x;
  if (pt.m_x == 1) return false;
  if ((pt.m_x | pt.m_y) == 0) pt.m_x = 1;
  return true;
}

struct DMRoomBuf : public RoomBuf
{
  struct DMDoor
  {
    DMRoomBuf const* m_room;
    Point<int16_t>   m_dir;

    DMDoor() : m_room(), m_dir() {};
    DMDoor( DMRoomBuf const* _room, Point<int16_t> const& _dir ) : m_room( _room ), m_dir( _dir ) {}
    ~DMDoor() {}
    
    bool next()
    {
      for (Point<int16_t> dir = m_dir; nextdir( dir );) {
        Point<int16_t> nroom = m_room->m_loc + dir;
        if (not DMRoomBuf::valid( nroom )) continue;
        m_dir = dir;
        return true;
      }
      return false;
    }
  
    Point<int32_t> getpos() const;
    
    Gate destination() const
    {
      // compute destination room and position (with destination door)
      DMRoomBuf* r = new DMRoomBuf( m_room->m_loc + m_dir );
      return Gate( r, DMDoor( r, -m_dir ).getpos() );
    }
  };
  
  Point<int16_t>        m_loc;
  
  DMRoomBuf( Point<int16_t> const& _loc ) : m_loc(_loc) {}
  DMRoomBuf( DMRoomBuf const& _room ) { throw "NoNoNo"; }
  virtual ~DMRoomBuf() {};
  
  int cmp( RoomBuf const& _rb ) const
  {
    Point<int16_t> const& a = m_loc;
    Point<int16_t> const& b = dynamic_cast<DMRoomBuf const&>( _rb ).m_loc;
    if (a.m_x < b.m_x) return -1;
    if (a.m_x > b.m_x) return +1;
    if (a.m_y < b.m_y) return -1;
    if (a.m_y > b.m_y) return +1;
    return 0;
  }
  
  DMDoor                firstdoor() const { return DMDoor( this, Point<int16_t>() ); }
  void                  process( Action& _action ) const;
  void                  dispose() const { delete this; }
  static const int rad = 4;
  static bool           valid( Point<int16_t> const& pos ) {
    if (pos.m_x == rad and pos.m_y == (rad-1)) return true;
    return (pos.m_x >= 0 and pos.m_y >= 0 and pos.m_x < rad and pos.m_y < rad);
  }
  std::string           getname() const;
  static Gate           start_incoming();
};

#endif /*__DMMAP_HH__*/
