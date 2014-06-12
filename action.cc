#include <action.hh>
#include <gallery.hh>
#include <SDL/SDL.h>
#include <iostream>
#include <memory>

void
Control::collect()
{
  for (SDL_Event event; SDL_PollEvent( &event );)
    {
      if      (event.type == SDL_QUIT)    throw Quit();
      else if (event.type == SDL_KEYDOWN) m_keys.set( event.key.keysym.sym, true );
      else if (event.type == SDL_KEYUP)   m_keys.reset( event.key.keysym.sym );
    }
}

Action::Action( SDL_Surface* _screen )
  : m_screen( _screen ),
    m_next_ticks( SDL_GetTicks() + FramePeriod ),
    m_pos(), m_room(), m_story()
{
}

void Action::run()
{
  for (;;) {
    m_control.collect();
    m_story.append( m_room, m_pos, m_control.fires() );
    
    if (m_control.picks()) {
      m_control.picked();
      SDL_Surface* thumb = SDL_DisplayFormatAlpha( m_screen );
      image_apply( Grayify(), thumb );
      m_story.newbwd( m_story.eoa, m_room, m_pos, thumb );
    }
    
    if (m_control.jumps()) {
      this->jump();
    }
    
    else {
      Room curroom( m_room );
      Point curpos( m_pos );
      
      curroom->process( *this );
            
      this->blit( curpos, gallery::hero );
    
      for (TimeLine *tl = m_story.firstghost(), *eotl = m_story.active; tl != eotl; tl = tl->fwd())
        {
          Point gpos;
          bool gfire;
          if (not tl->locate( m_story.eoa-1, curroom, gpos, gfire )) continue;
          this->blit( gpos, gallery::gray_ghost );
        }
    
      draw_timebar( tbs_point );
    
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
    beg = ((640-8)*m_story.boa/(m_story.eot-m_story.bot))+0;
  else
    beg = ((640-8)*m_story.eoa/(m_story.eot-m_story.bot))+0;
  end = ((640-8)*m_story.eoa/(m_story.eot-m_story.bot))+8;
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
        this->blit( Point(320, 192), slide );
        this->flipandwait();
      }
    SDL_FreeSurface( begin );
    SDL_FreeSurface( slide );
  }
    
  m_story.active->thumb( thumb );
  // std::string filename = cfmt( "jump%d.bmp", ++count );
  // SDL_SaveBMP( thumb, filename.c_str() );
    
  // typedef std::list<TimeLine*> TLs;
  // TLs tls;
  // TLs::iterator active;
  // for (Story::iterator src = m_story.begin(); src != m_story.end(); ++src) {
  //   TimeLine* tlp = &*src;
  //   tls.push_back( tlp );
  //   if (tlp == m_timeline) { active = tls.end(); --active; }
  // }
    
  while (true) {
    SDL_Surface* curthumb = m_story.active->m_thumb;
    this->blit( Point(320, 192), curthumb );
    draw_timebar( tbs_full );
    this->flipandwait();
    m_control.collect();
    if (m_control.picks()) { m_control.picked(); break; }
    if (m_story.active->single()) continue;
    int dir = 0;
    std::auto_ptr<TimeLine> aptl;
    if      (m_control.right())  { m_story.movfwd(); dir = -640; }
    else if (m_control.left())   { m_story.movbwd(); dir = +640; }
    else if (m_control.delfwd()) { aptl.reset( m_story.popfwd() ); dir = -640; }
    else if (m_control.delbwd()) { aptl.reset( m_story.popbwd() ); dir = +640; }
    else { continue; }
    SDL_Surface* nxtthumb = m_story.active->m_thumb;
      
    for (int idx = 0; idx < 24; ++idx) {
      Point cur( 320+((idx*dir)/24), 192 );
      Point nxt( 320+(((idx - 24)*dir)/24), 192 );
        
      this->blit( cur, curthumb );
      this->blit( nxt, nxtthumb );
        
      this->flipandwait();
    }
  }
  // date_t bounds[2];
  // m_timeline->getbounds( bounds );
  m_story.active->restore_state( m_pos, m_room );
  std::cerr << "Jumping to room: " << m_room->getname() << ".\n";
  if (m_control.shift())
    {
      m_story.newbwd( m_story.eoa, m_room, m_pos, 0 );
      m_story.movbwd();
    }
  else
    {
      SDL_FreeSurface( m_story.active->thumb( 0 ) );
      if (m_control.alt()) {
        /*fast forward to end of time*/
        m_story.forward();
      }
    }
  m_story.active->update_usetime();
}

void
Action::blit( Point const& _pos, SDL_Surface* _src )
{
  SDL_Rect offset;
  (_pos - (Point(_src->w, _src->h)/2)).pull( offset );
  
  SDL_BlitSurface( _src, NULL, m_screen, &offset );
}

void
Action::endstats( std::ostream& _sink )
{
  _sink << "Active records: " << m_story.record_count << ".\n";
}
