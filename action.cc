#include <action.hh>
#include <gallery.hh>
#include <SDL/SDL.h>
#include <SDL/SDL_keysym.h>
#include <iostream>
#include <memory>
#include <cmath>

Point<float>
PlayerInterface::motion() const
{
  int hor = int(cmds[Right])-int(cmds[Left]);
  int ver = int(cmds[Down])-int(cmds[Up]);
  float scale = (hor & ver) ? M_SQRT1_2 : 1.0;
  return Point<float>( hor*scale, ver*scale );
}

void
PlayerInterface::collect()
{
  for (SDL_Event event; SDL_PollEvent( &event );)
    {
      if      (event.type == SDL_QUIT)    throw Quit();
      else if ((event.type == SDL_KEYDOWN) or (event.type == SDL_KEYUP))
        {
          bool active = (event.type == SDL_KEYDOWN);
          switch (event.key.keysym.sym) {
          case SDLK_RETURN: case SDLK_KP_ENTER: cmds.set( Branch, active ); cmds.set( Select, active ); break;
          case SDLK_SPACE:                      cmds.set( Fire, active ); break;
          case SDLK_ESCAPE:                     cmds.set( Jump, active ); break;
          case SDLK_RIGHT:                      cmds.set( Right, active ); break;
          case SDLK_LEFT:                       cmds.set( Left, active ); break;
          case SDLK_DOWN:                       cmds.set( Down, active ); break;
          case SDLK_UP:                         cmds.set( Up, active ); break;
          case SDLK_DELETE:                     cmds.set( DelFwd, active ); break;
          case SDLK_BACKSPACE:                  cmds.set( DelBwd, active ); cmds.set( Debug, active ); break;
          case SDLK_LSHIFT: case SDLK_RSHIFT:   cmds.set( Shift, active ); break;
          case SDLK_LALT: case SDLK_RALT:       cmds.set( Alt, active ); break;
          default: break;
          }
        }
    }
}

Action::Action( SDL_Surface* _screen )
  : m_screen( _screen ), m_scratch( SDL_DisplayFormatAlpha( _screen ) ),
    m_next_ticks( SDL_GetTicks() + FramePeriod ), m_pos(),
    m_story(), m_room()
{
  image_apply( Fill<0xff,0xff,0xff,0x0>(), m_scratch );
}

Action::~Action()
{
  SDL_FreeSurface( m_scratch );
}

void Action::run()
{
  for (;;) {
    m_control.collect();
    m_story.append( m_room, m_pos, fires() );
    
    if (m_control.cmds[PlayerInterface::DelBwd]) {
      std::cout << "position: {" << m_pos.m_x << ',' << m_pos.m_y << "}\n";
    }
    
    if (m_control.cmds[PlayerInterface::Jump]) {
      this->jump();
    }
    
    else {
      Room curroom( m_room );
      
      m_origin = m_pos;
      curroom->process( *this );
      cutmotion( Point<float>( -4, -4 ), Point<float>( VideoConfig::width+4, -4 ) );
      cutmotion( Point<float>( VideoConfig::width+4, -4 ), Point<float>( VideoConfig::width+4, VideoConfig::height+4 ) );
      cutmotion( Point<float>( VideoConfig::width+4, VideoConfig::height+4 ), Point<float>( -4, VideoConfig::height+4 ) );
      cutmotion( Point<float>( -4, VideoConfig::height+4 ), Point<float>( -4, -4 ) );
      this->blit( m_origin.rebind<int32_t>(), gallery::hero );
      
      for (TimeLine *tl = m_story.firstghost(), *eotl = m_story.active; tl != eotl; tl = tl->fwd())
        {
          date_t now = m_story.now(), gdate = now;
          Ghost ghost(curroom);
          if (tl->match( now, ghost ))
            this->blit( ghost.pos, gallery::gray_ghost );
          if (tl->locate( gdate, ghost ))
            this->blit( ghost.pos, (now < gdate) ? gallery::blue_ghost : gallery::red_ghost ); 
        }
      
      if (m_control.getandclear(PlayerInterface::Branch)) {
        SDL_Surface* thumb = SDL_DisplayFormatAlpha( m_screen );
        image_apply( Grayify(), thumb );
        m_story.newbwd( m_room, m_pos, thumb );
      }
    
      draw_timebar( tbs_full );
      
      this->flipandwait();
    }
  }
}

void
Action::draw_timebar( Action::timebar_style tbs )
{
  if ((m_story.eoa > m_story.eot) or (m_story.boa < m_story.bot)) return;
  int beg, end;
  if (tbs == tbs_full)
    beg = ((640-8)*(m_story.boa-m_story.bot)/(m_story.eot-m_story.bot))+0;
  else
    beg = ((640-8)*(m_story.eoa-m_story.bot)/(m_story.eot-m_story.bot))+0;
  end = ((640-8)*(m_story.eoa-m_story.bot)/(m_story.eot-m_story.bot))+8;
  SDL_Rect bar;
  bar.x = beg;
  bar.y = 384-8;
  bar.w=end-beg;
  bar.h=8;
  // = { .x=int16_t(beg), .y=int16_t(384-8), .w=int16_t(end-beg), .h=int16_t(8) };
  SDL_FillRect(m_screen, &bar, SDL_MapRGB(m_screen->format, 0, 0, 0));
  bar.x += 2;
  bar.y += 2;
  bar.w -= 4;
  bar.h -= 4;
  SDL_FillRect(m_screen, &bar, SDL_MapRGB(m_screen->format, 255, 255, 255));
    
  // bar.x = 160; bar.w = 320;
  // SDL_FillRect(m_screen, &bar, SDL_MapRGB(m_screen->format, 255, 255, 255));
}

void
Action::flipandwait()
{
  if (SDL_Flip( m_screen ) == -1)
    {
      std::cerr << "Can't flip m_screen: " << SDL_GetError() << '\n';
      throw "Unexpected SDL error";
    }
  int now_ticks;
  while ((now_ticks = SDL_GetTicks()) < m_next_ticks)
    {
      int duration = m_next_ticks - now_ticks;
      //std::cerr << "waiting for " << duration << "ms.\n";
      SDL_Delay( duration );
    }
  m_next_ticks = now_ticks + FramePeriod;
}

void
Action::jump()
{
  // int oldh = m_screen->h;
  // m_screen->h = 384;
  std::cerr << "Jumping from room: " << m_room->getname() << ".\n";
  m_story.active->compress();
    
  SDL_Surface* thumb = SDL_DisplayFormatAlpha( m_screen );
  this->flipandwait();
  {
    // Animate thumb apparition
    SDL_Surface* begin = SDL_DisplayFormatAlpha( thumb );
    SDL_Surface* slide = SDL_DisplayFormatAlpha( thumb );
    image_apply( Grayify(), thumb );
    // m_screen->h = oldh;
    
    for (int idx = 0; idx < 16; ++idx)
      {
        image_apply( Fade(slide,begin,thumb,double(idx)/16.), slide );
        this->blit( slide );
        this->flipandwait();
      }
    SDL_FreeSurface( begin );
    SDL_FreeSurface( slide );
  }
  
  m_story.active->setthumb( thumb );
  m_story.writebounds( std::cerr << "Bound before jump: " ) << std::endl;
    
  while (true) {
    SDL_Surface* curthumb = m_story.active->getthumb();
    this->blit( curthumb );
    draw_timebar( tbs_full );
    this->flipandwait();
    m_control.collect();
    if (m_control.getandclear(PlayerInterface::Select)) break;
    if (m_story.active->single()) continue;
    int dir = 0;
    std::auto_ptr<TimeLine> aptl;
    if      (m_control.cmds[PlayerInterface::Right])  { m_story.movfwd(); dir = -640; }
    else if (m_control.cmds[PlayerInterface::Left])   { m_story.movbwd(); dir = +640; }
    else if (m_control.cmds[PlayerInterface::DelFwd]) { aptl.reset( m_story.popfwd() ); dir = -640; }
    else if (m_control.cmds[PlayerInterface::DelBwd]) { aptl.reset( m_story.popbwd() ); dir = +640; }
    else { continue; }
    SDL_Surface* nxtthumb = m_story.active->getthumb();
      
    for (int idx = 0; idx < 24; ++idx) {
      Point<int32_t> cur( 320+((idx*dir)/24), 192 );
      Point<int32_t> nxt( 320+(((idx - 24)*dir)/24), 192 );
        
      this->blit( cur, curthumb );
      this->blit( nxt, nxtthumb );
        
      this->flipandwait();
    }
  }
  m_story.writebounds( std::cerr << "Bound after jump: " ) << std::endl;
  m_story.active->restore_state( m_pos, m_room );
  std::cerr << "Jumping to room: " << m_room->getname() << ".\n";
  if (m_control.cmds[PlayerInterface::Shift])
    {
      m_story.newbwd( m_room, m_pos, 0 );
      m_story.movbwd();
    }
  else
    {
      m_story.active->setthumb( 0 );
      if (m_control.cmds[PlayerInterface::Alt]) {
        /*fast forward to end of time*/
        m_story.forward();
      }
    }
  m_story.active->update_usetime();
}

void
Action::blit( Point<int32_t> const& _pos, Pixel* _pixels, uintptr_t _width, uintptr_t _height )
{
  void* pixels = reinterpret_cast<void*>(_pixels);
  int height = _height, width = _width;
  Uint16 pitch = _width*4;
  std::swap( pixels, m_scratch->pixels );
  std::swap( width, m_scratch->w );
  std::swap( height, m_scratch->h );
  std::swap( pitch, m_scratch->pitch );
  blit( _pos, m_scratch );
  std::swap( pixels, m_scratch->pixels );
  std::swap( width, m_scratch->w );
  std::swap( height, m_scratch->h );
  std::swap( pitch, m_scratch->pitch );
}

void
Action::blit( Point<int32_t> const& _pos, SDL_Surface* _src )
{
  SDL_Rect offset;
  (_pos - (Point<int32_t>(_src->w, _src->h)/2)).pull( offset );
  
  SDL_BlitSurface( _src, NULL, m_screen, &offset );
}

void
Action::blit( SDL_Surface* _src )
{
  SDL_Rect offset;
  Point<int32_t>(0,0).pull( offset );
  
  SDL_BlitSurface( _src, NULL, m_screen, &offset );
}

void
Action::endstats( std::ostream& _sink )
{
  _sink << "Active records: " << m_story.record_count << ".\n";
}

void
Action::cutmotion( Point<float> const& w1, Point<float> const& w2 )
{
  // computing intersection
  Point<float> const& m1( m_origin );
  Point<float> const& m2( m_pos );
  Point<float> wd = w2 - w1;
  Point<float> md = m2 - m1;
  float det = wd*!md;
  if (fabs(det) < 1e-6) return;
  Point<float>  x = md*(w2*!w1)/det - wd*(m2*!m1)/det;
  
  // Checking if intersection lies in between m1..m2 and w1..w2 
  if ((m2-x)*(m1-x) > 0) return;
  if ((w2-x)*(w1-x) > 0) return;
  
  // prevent collision
  if ((m1-x).sqnorm() > 8)
    m_pos = (x*2 + m1)/3;
  else
    m_pos = m1;
}
