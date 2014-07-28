#include <epmap.hh>
#include <action.hh>
#include <SDL/SDL.h>
#include <iostream>
#include <sstream>
#include <gallery.hh>
#include <cmath>

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
    _action.blit( Point( 64, 96+64*door ), code.bit( door ) ? gallery::shiny_starfish : gallery::starfish );
  
  static Point const exitpos( 480, 192 );
  bool fishexit = (_action.m_pos - exitpos).m2() <= 24*24;
  
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
    _action.blit( repulsor::surface( _action.now() ) );
    _action.biasedmotion( 16, repulsor::motion( _action.m_pos, _action.now() ) );
  }
  _action.blit( exitpos, fishexit ? gallery::shiny_starfish : gallery::starfish );
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

SDL_Surface* repulsor::__surface__ = 0;
ImageStore repulsor::__is__( repulsor::__init__, repulsor::__exit__ );

void repulsor::__init__( SDL_Surface* _screen )
{
  // SDL_Surface* tmp = SDL_DisplayFormatAlpha( _screen );
  // tmp->w = 256;
  // tmp->h = 256;
  // __surface__ = SDL_DisplayFormatAlpha( tmp );
  // SDL_FreeSurface( tmp );
  __surface__ = SDL_DisplayFormatAlpha( _screen );
  image_apply( Fill<0xff,0xff,0xff,0x0>(), __surface__ );
  
  for (uintptr_t y = 0; y < 384; ++y) {
    for (uintptr_t x = 0; x < 640; ++x) {
      double dx = (479.5 - x);
      double dy = (191.5 - y);
      double norm = dx*dx + dy*dy;
      hf.table[y][x] = (norm*sqrt( norm )/65536)*(1<<16);
    }
  }
}

HydroMap<640,384> repulsor::hf;

void repulsor::__exit__()
{
  SDL_FreeSurface( __surface__ );
}

SDL_Surface*
repulsor::surface( uintptr_t date )
{
  uint8_t* img = (uint8_t*)__surface__->pixels;
  for (int y = 0, ystop = __surface__->h; y < ystop; ++y) {
    uint8_t* line = &img[y*__surface__->w*4];
    for (int x = 0, xstop = __surface__->w; x < xstop; ++x) {
      uint8_t* alpha = &line[x*4+3];
      int32_t value = hf.table[y][x];
      if ((value < 0) or (value > 32*(1<<16))) { *alpha = 0; continue; }
      uint32_t lum = ((hf.table[y][x] - date*(1<<16)) >> 10) & 0x1ff;
      if (lum >= 0x100) { lum = 0x1ff - lum; }
      int32_t decay = 256-value*8/(1<<16);
      *alpha = lum*decay >> 8;
    }
  }
  return __surface__;
}

Point
repulsor::motion( Point const& pos, uintptr_t date )
{
  double dx, dy;
  if (not hf.getdx( pos, dx )) return Point();
  if (not hf.getdy( pos, dy )) return Point();
  double module = sqrt( dx*dx + dy*dy );
  if (module < 1e-6) return Point();
  dx /= module; dy /= module;
  module += 0.025;
  dx /= module; dy /= module;
  return Point( dx, dy );
}
