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
  Point<float>    motion() const {
    int hor = int(m_keys[SDLK_RIGHT])-int(m_keys[SDLK_LEFT]);
    int ver = int(m_keys[SDLK_DOWN])-int(m_keys[SDLK_UP]);
    float scale = (hor & ver) ? M_SQRT1_2 : 1.0;
    return Point<float>( hor*scale, ver*scale );
  }
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
  SDL_Surface* screen() { return m_screen; }

private:  
  SDL_Surface*      m_screen;
  SDL_Surface*      m_scratch;
  static const int  FramePeriod = 40;
  static const int  FastMotion = 16;
  static const int  SlowMotion = 1;
  int               m_next_ticks;
  Control           m_control;
  Point<float>      m_pos, m_origin;
  
public:
  // Engine
  void moveto( Gate const& gate ) { m_room = gate.room; m_pos = Point<float>( gate.pos.m_x, gate.pos.m_y ); }
  void normalmotion() { m_pos += m_control.motion()*(m_control.shift() ? SlowMotion : FastMotion); }
  void moremotion( Point<float> const& _delta ) { m_pos += _delta; }
  void cutmotion( Point<float> const& w1, Point<float> const& w2 );
  Point<float> const& pos() const { return m_origin; }
  
  date_t            now() const { return m_story.now(); }
  
  Story             m_story;
  Room              m_room;
  
  void endstats( std::ostream& _sink );
};

#endif /* __ACTION_HH__ */
