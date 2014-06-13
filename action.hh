#ifndef __ACTION_HH__
#define __ACTION_HH__

#include <video.hh>
#include <map.hh>
#include <timeline.hh>
#include <SDL/SDL_keysym.h>
#include <bitset>
#include <iosfwd>

struct Control
{
  std::bitset<SDLK_LAST> m_keys;
  
  struct Quit {};
  
  Control() {}
  
  void collect();
  
  bool   picks() const { return m_keys[SDLK_RETURN] or m_keys[SDLK_KP_ENTER]; }
  void   picked() { m_keys[SDLK_RETURN] = false; m_keys[SDLK_KP_ENTER] = false; }
  bool   fires() const { return m_keys[SDLK_SPACE]; }
  void   fired() { m_keys[SDLK_SPACE] = false; }
  bool   jumps() const { return m_keys[SDLK_ESCAPE] or m_keys[SDLK_TAB]; }
  Point  motion() const { return Point( int(m_keys[SDLK_RIGHT])-int(m_keys[SDLK_LEFT]), int(m_keys[SDLK_DOWN])-int(m_keys[SDLK_UP]) ); }
  bool   right() const { return m_keys[SDLK_RIGHT]; }
  bool   left() const { return m_keys[SDLK_LEFT]; }
  bool   delfwd() const { return m_keys[SDLK_DELETE]; }
  bool   delbwd() const { return m_keys[SDLK_BACKSPACE]; }
  bool   shift() const { return m_keys[SDLK_LSHIFT] or m_keys[SDLK_RSHIFT]; }
  bool   alt() const { return m_keys[SDLK_LALT] or m_keys[SDLK_RALT]; }
};

struct Action
{
  Action( SDL_Surface* _screen );
  
  // Gamer interaction
  void run();
  enum timebar_style { tbs_full = 0, tbs_point };
  void draw_timebar( timebar_style tbs );
  void flipandwait();
  void jump();
  bool fires() const { return m_control.fires(); }
  void fired() { m_control.fired(); }
  void blit( Point const& _pos, SDL_Surface* _src );
  void blit( SDL_Surface* _src );

private:  
  SDL_Surface*      m_screen;
  static const int  FramePeriod = 40;
  int               m_next_ticks;
  Control           m_control;
  
public:
  // Engine
  void moveto( Gate const& gate ) { m_room = gate.room; m_pos = gate.pos; }
  void normalmotion() { m_pos += m_control.motion()*10; }
  
  Story             m_story;
  Room              m_room;
  Point             m_pos;
  
  void endstats( std::ostream& _sink );
};

#endif /* __ACTION_HH__ */
