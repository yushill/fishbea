#ifndef __SLIDEROOM_HH__
#define __SLIDEROOM_HH__

#include <map.hh>

struct SlideRoomBuf : public RoomBuf
{
  SlideRoomBuf() {}
  SlideRoomBuf( SlideRoomBuf const& _room ) { throw "NoNoNo"; }
  virtual ~SlideRoomBuf() {}
  
  void                  dispose() const { delete this; }
  std::string           getname() const { return "SlideRoom"; }
  void                  process( Action& _action ) const;
  int                   cmp( RoomBuf const& _rb ) const { return 0; }
  
  static Gate           start_incoming();
  static Gate           end_upcoming();
};

#endif /*__SLIDEROOM_HH__*/
