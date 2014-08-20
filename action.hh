#ifndef __ACTION_HH__
#define __ACTION_HH__

#include <video.hh>
#include <map.hh>
#include <timeline.hh>
#include <SDL/SDL_keysym.h>
#include <bitset>
#include <iosfwd>
#include <cmath>

struct Control
{
  std::bitset<SDLK_LAST> m_keys;
  
  struct Quit {};
  
  Control() {}
  
  void            collect();
  
  bool            picks() const { return m_keys[SDLK_RETURN] or m_keys[SDLK_KP_ENTER]; }
  void            picked() { m_keys[SDLK_RETURN] = false; m_keys[SDLK_KP_ENTER] = false; }
  bool            fires() const { return m_keys[SDLK_SPACE]; }
  void            fired() { m_keys[SDLK_SPACE] = false; }
  bool            jumps() const { return m_keys[SDLK_ESCAPE] or m_keys[SDLK_TAB]; }
  template <typename PT, typename INT>
  void            move( PT& _, INT speed ) const
  {
    _.m_x += speed*(int32_t(m_keys[SDLK_RIGHT])-int32_t(m_keys[SDLK_LEFT]));
    _.m_y += speed*(int32_t(m_keys[SDLK_DOWN])-int32_t(m_keys[SDLK_UP]));
  }
  int             horizontal() const { return int(m_keys[SDLK_RIGHT])-int(m_keys[SDLK_LEFT]); }
  int             vertical() const { return int(m_keys[SDLK_DOWN])-int(m_keys[SDLK_UP]); }
  bool            right() const { return m_keys[SDLK_RIGHT]; }
  bool            left() const { return m_keys[SDLK_LEFT]; }
  bool            delfwd() const { return m_keys[SDLK_DELETE]; }
  bool            delbwd() const { return m_keys[SDLK_BACKSPACE]; }
  bool            shift() const { return m_keys[SDLK_LSHIFT] or m_keys[SDLK_RSHIFT]; }
  bool            alt() const { return m_keys[SDLK_LALT] or m_keys[SDLK_RALT]; }
};

struct Action
{
  Action( SDL_Surface* _screen );
  ~Action();
  
  // Gamer interaction
  void run();
  enum timebar_style { tbs_full = 0, tbs_point };
  void draw_timebar( timebar_style tbs );
  void flipandwait();
  void jump();
  bool fires() const { return m_control.fires(); }
  void fired() { m_control.fired(); }
  void blit( Point<int32_t> const& _pos, SDL_Surface* _src );
  void blit( SDL_Surface* _src );
  SDL_Surface* scratch( SDL_Surface* _scratch=0 ) { return m_scratch; }

private:  
  SDL_Surface*      m_screen;
  SDL_Surface*      m_scratch;
  static const int  FramePeriod = 40;
  static const int  FastMotion = 16;
  static const int  SlowMotion = 1;
  int               m_next_ticks;
  Control           m_control;
  
public:
  // Engine
  void moveto( Gate const& gate ) { m_room = gate.room; m_pos = Point<float>( gate.pos.m_x, gate.pos.m_y ); }
  void normalmotion()
  {
    int32_t hor = m_control.horizontal();
    int32_t ver = m_control.vertical();
    float speed = m_control.shift() ? SlowMotion : FastMotion;
    if (hor & ver) speed *= M_SQRT1_2;
    m_pos.m_x += speed*hor;
    m_pos.m_y += speed*ver;
  }
  void biasedmotion( Point<float> const& _bias )
  {
    normalmotion();
    m_pos += _bias;
  }
  
  date_t            now() const { return m_story.now(); }
  
  Story             m_story;
  Room              m_room;
  Point<float>      m_pos;
  
  void endstats( std::ostream& _sink );
};

#endif /* __ACTION_HH__ */
