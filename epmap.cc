#include <epmap.hh>
#include <action.hh>
#include <iostream>
#include <sstream>
#include <gallery.hh>

RecycleHeap<EPRoomBuf> EPRoomBuf::heap;

struct Code
{
  Room room;
  uint32_t value;
  Code( Room _room ) : room(_room), value(0) {}
  bool match( Room _room, Point const& _pos, bool fire )
  {
    if (_room != room or not fire) return false;
    int idx = (_pos.m_y >> 6) - 1;
    if (idx < 0 or idx >= 4) return false;
    if ((Point( _pos.m_x, _pos.m_y & 63 ) - Point(64,32)).m2() > 24*24) return false;
    value |= (1 << idx);
    return false;
  }
  bool bit( int idx ) { return (value >> idx) & 1; }
};

void
EPRoomBuf::process( Action& _action ) const
{
  // Collision items precomputed from scene rendering
  Code code( this );
  {
    TimeLine *tl = _action.m_story.active, *eotl = _action.m_story.active;
    do { tl->match( _action.m_story.eoa-1, code ); } while ((tl = tl->fwd()) != eotl);
  }  

  // Scene Draw
  _action.blit( Point(320, 192), gallery::classic_bg );
  for (int door = 0; door < 4; ++door )
    _action.blit( Point( 64, 96+64*door ), code.bit( door ) ? gallery::shiny_shell : gallery::shell );
  
  if (code.value == m_code) {
    static Point const exit( 576, 192 );
    // draw exit
    bool fishexit = (_action.m_pos - exit).m2() <= 24*24;
    _action.blit( exit, fishexit ? gallery::shiny_shell : gallery::shell );
    if (_action.m_control.fires() and fishexit)
      {
        _action.moveto( this->end_upcoming() );
        std::cerr << "Entering room: " << _action.m_room->getname() << ".\n";
        _action.m_control.fired();
        return;
      }
  }
  
  _action.m_pos += _action.m_control.motion()*10;
}

RoomBuf const*
EPRoomBuf::firstroom()
{
  // EPRoomBuf* res = EPRoomBuf::heap.allocate();
  // new (res) EPRoomBuf( Point() );
  // return res;
  return 0;
}

std::string
EPRoomBuf::getname() const
{
  std::ostringstream oss;
  oss << "EPRoom[" << std::hex << m_code << "]";
  return oss.str();
}

