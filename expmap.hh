#ifndef __EXPMAP_HH__
#define __EXPMAP_HH__

#include <map.hh>

struct ExpRoomBuf : public RoomBuf
{
  ExpRoomBuf() {}
  ExpRoomBuf( ExpRoomBuf const& _room ) { throw "NoNoNo"; }
  virtual ~ExpRoomBuf() {}
  
  void                  dispose() const { delete this; }
  std::string           getname() const { return "ExpRoom"; }
  void                  process( Action& _action ) const;
  int                   cmp( RoomBuf const& _rb ) const { return 0; }
  
  static Gate           start_incoming();
  static Gate           end_upcoming();
  
};

#endif /*__EXPMAP_HH__*/
