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
  
  static Point const exitpos( 576, 192 );
  Point exitgap = _action.m_pos - exitpos;
  int sqmodule = exitgap.m2();
  bool fishexit = sqmodule <= 24*24;
  _action.blit( exitpos, fishexit ? gallery::shiny_shell : gallery::shell );
  
  if (code.value == m_code) {
    // draw exit
    if (_action.fires() and fishexit)
      {
        _action.moveto( this->end_upcoming() );
        std::cerr << "Entering room: " << _action.m_room->getname() << ".\n";
        _action.fired();
        return;
      }
    else
      _action.normalmotion();
  } else {
    int const dev = (1 << 9);
    int psqm = dev + sqmodule;
    _action.biasedmotion( 16, exitgap*(float(16*dev*dev)/psqm/psqm) );
  }
  
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

