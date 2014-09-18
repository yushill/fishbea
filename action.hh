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
    virtual void init() = 0;
    virtual void exit() = 0;
    Store* next;
    static Store* pool;
    Store(): next( pool ) { pool = this; }
    void lstinit() { if (next) next->lstinit(); init(); }
    void lstexit() { exit(); if (next) next->lstexit(); }
  };
  
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

  enum timebar_style { tbs_full = 0, tbs_point };
  
  Action();
  ~Action();
  
  // Top control
  void              run();
  void              endstats( std::ostream& _sink );
  
  // Graphics
  void cornerblit( Point<int32_t> const& _pos, Pixel* data, uintptr_t width, uintptr_t height );
  template <uintptr_t WIDTH, uintptr_t HEIGHT>
  void centerblit( Point<int32_t> const& _pos, Pixel (&img)[HEIGHT][WIDTH] )
  { cornerblit( _pos - Point<int32_t>( WIDTH, HEIGHT )/2, &img[0][0], WIDTH, HEIGHT ); }
  template <uintptr_t WIDTH, uintptr_t HEIGHT>
  void cornerblit( Point<int32_t> const& _pos, Pixel (&img)[HEIGHT][WIDTH] )
  { cornerblit( _pos, &img[0][0], WIDTH, HEIGHT ); }
  
  void draw_timebar( timebar_style tbs );
  void flipandwait() { m_lastflip = GamerInterface::flipandwait( thescreen, m_lastflip + FramePeriod ); }
  
  Screen            thescreen;
  
  // Motion
  void moveto( Gate const& gate );
  void normalmotion() { m_pos += GamerInterface::motion( *this )*(cmds[Shift] ? SlowMotion : FastMotion); }
  void moremotion( Point<float> const& _delta ) { m_pos += _delta; }
  void cutmotion( Point<float> const& w1, Point<float> const& w2 );
  Point<float> const& pos() const { return m_origin; }
  
  // Interactions
  Story const&      story() const { return m_story; }
  date_t            now() const { return m_story.now(); }
  Cmds              cmds;
  
private:  
  void              jump();
  
  static const int  FramePeriod = 40;
  static const int  FastMotion = 16;
  static const int  SlowMotion = 1;
  int               m_lastflip;
  Story             m_story;
  Room              m_room;
  Point<float>      m_pos, m_origin;
};

#endif /* __ACTION_HH__ */
