#ifndef __DMMAP_HH__
#define __DMMAP_HH__

#include <map.hh>
#include <new>

struct DMRoomBuf : public RoomBuf
{
  struct DMDoor
  {
    DMRoomBuf const* m_room;
    Point            m_dir;

    DMDoor() : m_room(), m_dir() {};
    DMDoor( DMRoomBuf const* _room, Point const& _dir ) : m_room( _room ), m_dir( _dir ) {}
    ~DMDoor() {}
    
    bool next()
    {
      for (Point dir = m_dir; dir.nextdir();) {
        Point nroom = m_room->m_loc + dir;
        if (not DMRoomBuf::valid( nroom )) continue;
        m_dir = dir;
        return true;
      }
      return false;
    }
  
    Point getpos() const;
    
    Gate destination() const
    {
      // compute destination room and position (with destination door)
      DMRoomBuf* r = new DMRoomBuf( m_room->m_loc + m_dir );
      return Gate( r, DMDoor( r, -m_dir ).getpos() );
    }
  };
  
  Point                 m_loc;
  
  DMRoomBuf( Point const& _loc ) : m_loc(_loc) {}
  //DMRoomBuf( DMRoomBuf const& _room ) : m_loc(_room.m_loc) {}
  virtual ~DMRoomBuf() {};
  
  int cmp( RoomBuf const& _rb ) const
  {
    Point const& a = m_loc;
    Point const& b = dynamic_cast<DMRoomBuf const&>( _rb ).m_loc;
    if (a.m_x < b.m_x) return -1;
    if (a.m_x > b.m_x) return +1;
    if (a.m_y < b.m_y) return -1;
    if (a.m_y > b.m_y) return +1;
    return 0;
  }
  
  DMDoor                firstdoor() const { return DMDoor( this, Point() ); }
  void                  process( Action& _action ) const;
  void                  dispose() const { delete this; }
  static const int rad = 4;
  static bool           valid( Point const& pos ) {
    if (pos.m_x == rad and pos.m_y == (rad-1)) return true;
    return (pos.m_x >= 0 and pos.m_y >= 0 and pos.m_x < rad and pos.m_y < rad);
  }
  std::string           getname() const;
  static Gate           start_incoming();
};

#endif /*__DMMAP_HH__*/
