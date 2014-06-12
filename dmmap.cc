#include <dmmap.hh>
#include <action.hh>
#include <iostream>
#include <sstream>
#include <gallery.hh>

void
DMRoomBuf::process( Action& _action ) const
{
  // Collision items precomputed from scene rendering
  DMDoor active;
    
  // std::cerr << "Subject pos: " << m_x << "," << m_y << ".\n";
  // Scene Draw
  _action.blit( Point(320, 192), gallery::classic_bg );
  for (DMDoor door = this->firstdoor(); door.next(); )
    {
      Point pos = door.getpos();
      if ((pos - _action.m_pos).m2() > 24*24) {
        _action.blit( pos, gallery::shell );
      } else {
        _action.blit( pos, gallery::shiny_shell );
        active = door;
      }
    }
  // GameWorld interaction
    
  if (_action.m_control.fires())
    {
      if (active.m_room) {
        _action.moveto( active.destination() );
        std::cerr << "Entering room: " << _action.m_room->getname() << ".\n";
        _action.m_control.fired();
      }
    }
    
  else
    _action.m_pos += _action.m_control.motion()*10;
}

Gate
DMRoomBuf::start_incoming()
{
  return Gate( new DMRoomBuf( Point() ), Point(50,50) );
}

std::string
DMRoomBuf::getname() const
{
  std::ostringstream oss;
  oss << "DiamondRoom[" << m_loc.m_x << ", " << m_loc.m_y << "]";
  return oss.str();
}


Point shuffledir( int seed, Point const& _src )
{
  int index = ((_src.m_y - _src.m_x > 0) ? 1 : 0) | ((_src.m_y + _src.m_x > 0) ? 2 : 0);
  {
    int perms[4] = {0,1,2,3};
    for (int perm = 0; perm <= index; ++perm) {
      int pos = perm + (seed % (4-perm));
      if (pos > perm) { int tmp = perms[perm]; perms[perm] = perms[pos]; perms[pos] = tmp; }
    }
    index = perms[index];
  }
  int a = index & 1, b = (index >> 1) & 1;
  return Point( b - a, a + b - 1 );
}


