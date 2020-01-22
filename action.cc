#include <action.hh>
#include <gallery.hh>
#include <iostream>
#include <iomanip>
#include <memory>
#include <cmath>

Action::Store* Action::Store::pool = 0;

Action::Action()
  : m_lastflip( 0 ), m_story(), m_room(), m_pos()
{
  image_apply( Fill<0,0,0,0xff>(), thescreen.pixels );
  GamerInterface::init();
  Store::pool->lstinit();
}

Action::~Action()
{
  Store::pool->lstexit();
  GamerInterface::exit();
}

struct Ghost
{
  Room    room;
  Action& action;
  
  Ghost( Room _room, Action& _action ) : room(_room), action(_action) {}
  bool match( int when, Room _room, Point<int32_t> const& _pos, bool fire )
  {
    if (_room != room) return false;
    switch (when) {
    case -1:  action.centerblit( _pos, gallery::red_ghost ); break;
    case  0:  action.centerblit( _pos, gallery::gray_ghost ); break;
    case +1:  action.centerblit( _pos, gallery::blue_ghost ); break;
    };
    return false;
  }
};

std::ostream& operator << ( std::ostream& _sink, Point<float> const& _point )
{ _sink << "{" << std::setprecision(10) << _point.x << ',' << _point.y << "}"; return _sink; }

void Action::run()
{
  for (;;) {
    GamerInterface::collect( *this );
    m_story.append( m_room, m_pos, cmds[Fire] );
    
    static bool dbg = false;
    if (dbg or cmds[DelBwd]) {
      dbg = true;
      std::cerr << "position: " << m_pos << "\n";
    }
    
    if (cmds[Jump]) {
      this->jump();
    }
    
    else {
      Room curroom( m_room );
      
      m_collision = false;
      m_motion = (m_pos - m_origin)*0.5 + m_motion*0.5;
      m_origin = m_pos;
      curroom->process( *this );
      cutmotion( Point<float>( -4, -4 ), Point<float>( pixwidth(thescreen.pixels)+4, -4 ) );
      Point<float> precut( m_pos );
      cutmotion( Point<float>( pixwidth(thescreen.pixels)+4, -4 ), Point<float>( pixwidth(thescreen.pixels)+4, pixheight(thescreen.pixels)+4 ) );
      if (m_pos.x > (pixwidth(thescreen.pixels)+4)) {
        std::cerr << "Crossing: " << m_origin << ", " << m_pos << ", " << precut << std::endl;
      }
      cutmotion( Point<float>( pixwidth(thescreen.pixels)+4, pixheight(thescreen.pixels)+4 ), Point<float>( -4, pixheight(thescreen.pixels)+4 ) );
      cutmotion( Point<float>( -4, pixheight(thescreen.pixels)+4 ), Point<float>( -4, -4 ) );
      if (m_collision) m_motion = Point<float>( 0, 0 );
      this->centerblit( m_origin.rebind<int32_t>(), gallery::hero );
      
      Ghost ghost(curroom,*this);
      date_t now = m_story.now();
      for (TimeLine *tl = m_story.firstghost(), *eotl = m_story.active; tl != eotl; tl = tl->fwd())
        tl->find( now, ghost );
      
      if (cmds[Branch]) {
        cmds.reset();
        Screen* thumb = new Screen( thescreen );
        image_apply( Grayify(), thumb->pixels );
        m_story.newbwd( m_room, m_pos, thumb );
      }
    
      draw_timebar( tbs_full );
      
      this->flipandwait();
    }
  }
}

void
Action::endstats( std::ostream& _sink )
{
  _sink << "Active records: " << m_story.record_count << ".\n";
}

void
Action::cornerblit( Point<int32_t> const& _pos, Pixel* data, uintptr_t width, uintptr_t height )
{
  int32_t const ybeg = std::max( 0, -_pos.y );
  int32_t const yend = std::min( int32_t( height ), int32_t( pixheight(thescreen.pixels) )-_pos.y );
  int32_t const xbeg = std::max( 0, -_pos.x );
  int32_t const xend = std::min( int32_t( width ), int32_t( pixwidth(thescreen.pixels) )-_pos.x );
  
  for (int32_t y = ybeg; y < yend; ++y) {
    for (int32_t x = xbeg; x < xend; ++x) {
      Pixel& src = data[y*width+x];
      Pixel& dst = thescreen.pixels[y+_pos.y][x+_pos.x];
      unsigned int alpha = src.a + 1;
      unsigned int omega = 256 - src.a;
      dst.b = (omega*dst.b + alpha*src.b) >> 8;
      dst.g = (omega*dst.g + alpha*src.g) >> 8;
      dst.r = (omega*dst.r + alpha*src.r) >> 8;
    }
  }
}

void
Action::draw_timebar( Action::timebar_style tbs )
{
  if ((m_story.eoa > m_story.eot) or (m_story.boa < m_story.bot)) return;
  int32_t beg, end;
  if (tbs == tbs_full)
    beg = ((pixwidth(thescreen.pixels)-8)*(m_story.boa-m_story.bot)/(m_story.eot-m_story.bot))+0;
  else
    beg = ((pixwidth(thescreen.pixels)-8)*(m_story.eoa-m_story.bot)/(m_story.eot-m_story.bot))+0;
  end = ((pixwidth(thescreen.pixels)-8)*(m_story.eoa-m_story.bot)/(m_story.eot-m_story.bot))+8;
  
  int32_t top = pixheight( thescreen.pixels )-8;
  for (int32_t x = beg; x < end; ++x) {
    thescreen.pixels[top+0][x].set( 0, 0, 0, 0xff );
    thescreen.pixels[top+1][x].set( 0, 0, 0, 0xff );
    thescreen.pixels[top+6][x].set( 0, 0, 0, 0xff );
    thescreen.pixels[top+7][x].set( 0, 0, 0, 0xff );
  }
  for (int32_t y = 2; y < 6; ++y) {
    thescreen.pixels[top+y][beg+0].set( 0, 0, 0, 0xff );
    thescreen.pixels[top+y][beg+1].set( 0, 0, 0, 0xff );
    thescreen.pixels[top+y][end-2].set( 0, 0, 0, 0xff );
    thescreen.pixels[top+y][end-1].set( 0, 0, 0, 0xff );
  }
  for (int32_t y = top+2; y < int32_t( pixheight( thescreen.pixels ) - 2 ); ++y) {
    for (int32_t x = beg+2; x < end-2; ++x) {
      thescreen.pixels[y][x].set( 0xff, 0xff, 0xff, 0xff );
    }
  }
}

void 
Action::moveto( Gate const& gate )
{
  if (m_room != gate.room)
    std::cerr << "Entering room: " << gate.room->getname() << ".\n";
  m_room = gate.room;
  m_pos = Point<float>( gate.pos.x, gate.pos.y );
  m_origin = m_pos;
}

void
Action::cutmotion( Point<float> const& _w1, Point<float> const& _w2 )
{
  Point<double> w1( _w1.rebind<double>() ), w2( _w2.rebind<double>() );
  // computing intersection
  Point<double> m1( m_origin.rebind<double>() );
  Point<double> m2( m_pos.rebind<double>() );
  Point<double> wd = w2 - w1;
  Point<double> md = m2 - m1;
  double det = wd*!md;
  if (fabs(det) < 2e-38) return;
  Point<double>  x = (md*(w2*!w1) - wd*(m2*!m1))/det;
  
  // Checking if intersection lies in between m1..m2 and w1..w2 
  if ((m2-x)*(m1-x) > 0) return;
  if ((w2-x)*(w1-x) > 0) return;
  
  // prevent collision
  m_collision = true;
  if ((m1-x).sqnorm() > 8)
    m_pos = ((x*2 + m1)/3).rebind<float>();
  else
    m_pos = m_origin;
}

void
Action::jump()
{
  std::cerr << "Jumping from room: " << m_room->getname() << ".\n";
  m_story.active->compress();
    
  Screen* thumb = new Screen( thescreen );
  this->flipandwait();
  {
    // Animate thumb apparition
    Screen begin( thescreen );
    image_apply( Grayify(), thumb->pixels );
    
    for (int idx = 0; idx < 16; ++idx)
      {
        image_fade( thescreen.pixels, thumb->pixels, begin.pixels, idx*16 );
        this->flipandwait();
      }
  }
  
  m_story.active->setthumb( thumb );
  m_story.writebounds( std::cerr << "Bound before jump: " ) << std::endl;
  
  while (true) {
    Screen* curthumb = m_story.active->getthumb();
    this->cornerblit( Point<int32_t>(), curthumb->pixels );
    draw_timebar( tbs_full );
    this->flipandwait();
    GamerInterface::collect( *this );
    if (cmds[Select]) break;
    if (m_story.active->single()) continue;
    int dir = 0;
    std::unique_ptr<TimeLine> aptl;
    if      (cmds[Right])  { m_story.movfwd(); dir = -pixwidth(thescreen.pixels); }
    else if (cmds[Left])   { m_story.movbwd(); dir = +pixwidth(thescreen.pixels); }
    else if (cmds[DelFwd]) { aptl.reset( m_story.popfwd() ); dir = -pixwidth(thescreen.pixels); }
    else if (cmds[DelBwd]) { aptl.reset( m_story.popbwd() ); dir = +pixwidth(thescreen.pixels); }
    else { continue; }
    Screen* nxtthumb = m_story.active->getthumb();
      
    for (int idx = 0; idx < 24; ++idx) {
      Point<int32_t> cur( ((idx*dir)/24), 0 );
      Point<int32_t> nxt( (((idx - 24)*dir)/24), 0 );
        
      this->cornerblit( cur, curthumb->pixels );
      this->cornerblit( nxt, nxtthumb->pixels );
        
      this->flipandwait();
    }
  }
  m_story.writebounds( std::cerr << "Bound after jump: " ) << std::endl;
  m_story.active->restore_state( m_pos, m_room );
  m_motion = Point<float>();
  m_origin = m_pos;
  
  std::cerr << "Jumping to room: " << m_room->getname() << ".\n";
  if (cmds[Shift])
    {
      m_story.newbwd( m_room, m_pos, 0 );
      m_story.movbwd();
    }
  else
    {
      m_story.active->setthumb( 0 );
      if (cmds[Alt]) {
        /*fast forward to end of time*/
        m_story.forward();
      }
    }
  m_story.active->update_usetime();
  cmds.reset();
}

