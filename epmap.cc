#include <epmap.hh>
#include <action.hh>
#include <iostream>
#include <sstream>
#include <gallery.hh>

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
    do { tl->match( _action.m_story.now(), code ); } while ((tl = tl->fwd()) != eotl);
  }  

  // Scene Draw
  _action.blit( gallery::classic_bg );
  for (int door = 0; door < 4; ++door )
    _action.blit( Point( 64, 96+64*door ), code.bit( door ) ? gallery::shiny_shell : gallery::shell );
  
  if (code.value == m_code) {
    static Point const exit( 576, 192 );
    // draw exit
    bool fishexit = (_action.m_pos - exit).m2() <= 24*24;
    _action.blit( exit, fishexit ? gallery::shiny_shell : gallery::shell );
    if (_action.fires() and fishexit)
      {
        _action.moveto( this->end_upcoming() );
        std::cerr << "Entering room: " << _action.m_room->getname() << ".\n";
        _action.fired();
        return;
      }
  }
  
  _action.normalmotion();
}

Gate
EPRoomBuf::start_incoming()
{
  return Gate( new EPRoomBuf( 0xa ), Point(320, 192) );
}

std::string
EPRoomBuf::getname() const
{
  std::ostringstream oss;
  oss << "EPRoom[" << std::hex << m_code << "]";
  return oss.str();
}

