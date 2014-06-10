#include <epmap.hh>
#include <action.hh>
#include <iostream>
#include <sstream>
#include <gallery.hh>

RecycleHeap<EPRoomBuf> EPRoomBuf::heap;

bool
which_button( Point const& pos, int& _idx )
{
  int idx = (pos.m_y >> 6) - 1;
  if (idx < 0 or idx >= 4) return false;
  Point tmp( pos );
  tmp.m_y &= 63;
  if ((tmp - Point(64,32)).m2() > 24*24) return false;
  _idx = idx;
  return true;
}

void
EPRoomBuf::process( Action& _action ) const
{
  // Collision items precomputed from scene rendering
  int active;
  uint32_t code = 0;
  
  for (TimeLine *tl = _action.m_story.firstghost(), *eotl = _action.m_story.active; tl != eotl; tl = tl->fwd())
    {
      Point btn;
      bool fire;
      if (not tl->locate( _action.m_story.eoa-1, Room( this ), btn, fire ) or not fire) continue;
      int btnidx = -1;
      if (not which_button( btn, btnidx )) continue;
      code |= (1 << btnidx);
    }
    

  // Scene Draw
  _action.blit( Point(320, 192), gallery::classic_bg );
  for (int door = 0; door < 4; ++door )
    {
      Point pos( 64, 96+64*door );
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
      // if (active.m_room) {
      //   active.destination( _action.m_room, _action.m_pos );
      //   std::cerr << "Entering room: " << _action.m_room->getname() << ".\n";
      //   _action.m_control.fired();
      // }
    }
    
  else
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

