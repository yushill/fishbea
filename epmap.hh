#ifndef __EPMAP_HH__
#define __EPMAP_HH__

#include <map.hh>
#include <new>

struct EPRoomBuf : public RoomBuf
{
  EPRoomBuf( uint32_t _code ) : m_code(_code) {}
  EPRoomBuf( EPRoomBuf const& _room ) : m_code(_room.m_code) {}
  
  void                  dispose() const { heap.deallocate( this ); }
  std::string           getname() const;
  void                  process( Action& _action ) const;
  int                   cmp( RoomBuf const& _rb ) const
  {
    int diff = (m_code - dynamic_cast<EPRoomBuf const&>( _rb ).m_code);
    return (diff < 0) ? -1 : (diff > 0) ? 1 : 0;
  }
  
  uint32_t              m_code;
  static RoomBuf const* firstroom();
  static RecycleHeap<EPRoomBuf> heap;
  
  static Gate           end_upcoming();
};

#endif /*__EPMAP_HH__*/
