#ifndef __ACTION_HH__
#define __ACTION_HH__

#include <video.hh>
#include <map.hh>
#include <timeline.hh>
#include <bitset>
#include <iosfwd>

struct PlayerInterface
{
  struct Quit {};
  enum CmdCode {
    // Action
    Branch = 0, Fire, Jump,
    // Time Menu
    Right, Left, Down, Up, DelFwd, DelBwd, Select,
    // Mods
    Shift, Alt,
    Debug,
    CmdCodeCount
  };
  std::bitset<CmdCodeCount> cmds;
  
  PlayerInterface() {}
  
  void            collect();
  
  Point<float>    motion() const;
  bool getandclear(CmdCode code) { bool res = cmds[code]; cmds.reset(code); return res; }
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
  bool fires() const { return m_control.cmds[PlayerInterface::Fire]; }
  void fired() { m_control.cmds.reset(PlayerInterface::Fire); }
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
  PlayerInterface   m_control;
  Point<float>      m_pos, m_origin;
  
public:
  // Engine
  void moveto( Gate const& gate ) { m_room = gate.room; m_pos = Point<float>( gate.pos.m_x, gate.pos.m_y ); }
  void normalmotion() { m_pos += m_control.motion()*(m_control.cmds[PlayerInterface::Shift] ? SlowMotion : FastMotion); }
  void moremotion( Point<float> const& _delta ) { m_pos += _delta; }
  void cutmotion( Point<float> const& w1, Point<float> const& w2 );
  Point<float> const& pos() const { return m_origin; }
  
  date_t            now() const { return m_story.now(); }
  
  Story             m_story;
  Room              m_room;
  
  void endstats( std::ostream& _sink );
};

#endif /* __ACTION_HH__ */
