#ifndef __EPMAP_HH__
#define __EPMAP_HH__

#include <map.hh>

struct EPRoomBuf : public RoomBuf
{
  EPRoomBuf( uint32_t _code ) : m_code(_code) {}
  EPRoomBuf( EPRoomBuf const& _room ) { throw "NoNoNo"; }
  virtual ~EPRoomBuf() {}
  
  void                  dispose() const { delete this; }
  std::string           getname() const;
  void                  process( Action& _action ) const;
  int                   cmp( RoomBuf const& _rb ) const
  {
    int diff = (m_code - dynamic_cast<EPRoomBuf const&>( _rb ).m_code);
    return (diff < 0) ? -1 : (diff > 0) ? 1 : 0;
  }
  
  uint32_t              m_code;
  static Gate           start_incoming();
  static Gate           end_upcoming();
};

#include <video.hh>

template <uintptr_t WIDTH, uintptr_t HEIGHT>
struct HydroField
{
  int32_t table[HEIGHT][WIDTH];
  
  int32_t getdx( uintptr_t x, uintptr_t y, uintptr_t date )
  {
    if ((y<=0) or (y>=(HEIGHT-1)) or (x<=0) or (x>=(WIDTH-1))) return 0x80000000;
    int32_t vx0; if ((vx0 = table[y][x-1]) == 0x80000000) return 0x80000000;
    int32_t vx1; if ((vx1 = table[y][x-1]) == 0x80000000) return 0x80000000;
    return vx1-vx0;
  }
  int32_t getdy( uintptr_t x, uintptr_t y, uintptr_t date )
  {
    if ((y<=0) or (y>=(HEIGHT-1)) or (x<=0) or (x>=(WIDTH-1))) return 0x80000000;
    int32_t vy0; if ((vy0 = table[y][x-1]) == 0x80000000) return 0x80000000;
    int32_t vy1; if ((vy1 = table[y][x-1]) == 0x80000000) return 0x80000000;
    return vy1-vy0;
  }
};


struct repulsor
{
  static SDL_Surface* __surface__;
  static SDL_Surface* surface( uintptr_t date );
  static Point motion( Point const& exitgap, uintptr_t date );
  static void __init__( SDL_Surface* _screen );
  static void __exit__();
  static ImageStore __is__;
  
  static HydroField<640,384> hf;
};


#endif /*__EPMAP_HH__*/
