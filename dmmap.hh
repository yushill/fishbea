#ifndef __DMMAP_HH__
#define __DMMAP_HH__

#include <map.hh>

struct DMRoomBuf : public RoomBuf
{
  static int32_t const  MySide = 4;
  int32_t               m_index;
  
  explicit DMRoomBuf( int32_t _index ) : m_index( (_index+MySide*MySide)%(MySide*MySide) ) {}
  DMRoomBuf( DMRoomBuf const& _room ) { throw "NoNoNo"; }
  virtual ~DMRoomBuf() {};
  
  int cmp( RoomBuf const& _rb ) const
  {
    int32_t a = m_index, b = dynamic_cast<DMRoomBuf const&>( _rb ).m_index;
    return (a < b) ? -1 : (a == b) ? 0 : +1;
  }
  
  void                  process( Action& _action ) const;
  void                  dispose() const { delete this; }
  std::string           getname() const;
  
  static Gate           start_incoming();
  static Gate           end_incoming();
  static Gate           start_upcoming();
  static Gate           end_upcoming();
};

#endif /*__DMMAP_HH__*/
