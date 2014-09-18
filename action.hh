#ifndef __ACTION_HH__
#define __ACTION_HH__

#include <video.hh>
#include <gamerinterface.hh>
#include <map.hh>
#include <timeline.hh>
#include <bitset>
#include <iostream>
#include <iosfwd>

struct Action
{
  struct Store {
    typedef void (*init_method_t)();
    typedef void (*exit_method_t)();
    init_method_t init_method;
    exit_method_t exit_method;
    Store* next;
    static Store* pool;
    Store( init_method_t _init, exit_method_t _exit )
      : init_method( _init ), exit_method( _exit ), next( pool ) { pool = this; }
    void init() { if (next) next->init(); init_method(); }
    void exit() { exit_method(); if (next) next->exit(); }
  };

  Action();
  ~Action();
  
  // Game engine API
  void run();
  enum timebar_style { tbs_full = 0, tbs_point };
  void draw_timebar( timebar_style tbs );
  void flipandwait() { m_lastflip = GamerInterface::flipandwait( thescreen, m_lastflip + FramePeriod ); }
  void jump();
  bool fires() const { return cmds[Fire]; }
  void fired() { cmds.reset(Fire); }
  
  Screen            thescreen;
  void cornerblit( Point<int32_t> const& _pos, Pixel* data, uintptr_t width, uintptr_t height );
  template <uintptr_t WIDTH, uintptr_t HEIGHT>
  void centerblit( Point<int32_t> const& _pos, Pixel (&img)[HEIGHT][WIDTH] )
  { cornerblit( _pos - Point<int32_t>( WIDTH, HEIGHT )/2, &img[0][0], WIDTH, HEIGHT ); }
  template <uintptr_t WIDTH, uintptr_t HEIGHT>
  void cornerblit( Point<int32_t> const& _pos, Pixel (&img)[HEIGHT][WIDTH] )
  { cornerblit( _pos, &img[0][0], WIDTH, HEIGHT ); }

  int32_t           screen_width() { return pixwidth( thescreen.pixels ); }
  int32_t           screen_height() { return pixheight( thescreen.pixels ); }
  Point<int32_t>    screen_diag() { return Point<int32_t>( pixwidth( thescreen.pixels ), pixheight( thescreen.pixels ) ); }
  
private:  
  static const int  FramePeriod = 40;
  static const int  FastMotion = 16;
  static const int  SlowMotion = 1;
  int               m_lastflip;
  Point<float>      m_pos, m_origin;
  
  
public:
  // Engine
  void moveto( Gate const& gate ) { m_room = gate.room; m_pos = Point<float>( gate.pos.x, gate.pos.y ); }
  void normalmotion() { m_pos += GamerInterface::motion( *this )*(cmds[Shift] ? SlowMotion : FastMotion); }
  void moremotion( Point<float> const& _delta ) { m_pos += _delta; }
  void cutmotion( Point<float> const& w1, Point<float> const& w2 );
  Point<float> const& pos() const { return m_origin; }
  
  date_t            now() const { return m_story.now(); }
  
  Story             m_story;
  Room              m_room;
  
  void endstats( std::ostream& _sink );

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
  typedef std::bitset<CmdCodeCount> Cmds;
  Cmds cmds;
};

#endif /* __ACTION_HH__ */
