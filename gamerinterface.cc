#include <gamerinterface.hh>
#include <action.hh>
#include <stdexcept>
#include <SDL/SDL.h>
#include <SDL/SDL_keysym.h>
#include <cmath>

namespace GamerInterface
{
  struct SDLState
  {
    SDL_Surface* screen;
    SDL_Surface* scratch;
    
    SDLState() : screen(), scratch() {}
    
    void init() {
      // SDL environment setup
      if (SDL_Init( SDL_INIT_EVERYTHING ) == -1)
        throw std::runtime_error( std::string( "Can't init SDL: " ) + SDL_GetError() + '\n' );
      
      SDL_WM_SetCaption( "FishBea", NULL );
      screen = SDL_SetVideoMode( Screen::width, Screen::height, 32, SDL_SWSURFACE );
      if (not screen)
        throw std::runtime_error( std::string( "Can't set video mode: " ) + SDL_GetError() + "\n" );
      
      scratch = SDL_DisplayFormatAlpha( screen );
    }
    void exit() {
      SDL_FreeSurface( screen ); screen = 0;
      SDL_FreeSurface( scratch ); scratch = 0;
      SDL_Quit();
    }
  } sdl;
  
  void init() { sdl.init(); }
  void exit() { sdl.exit(); }
  
  void
  collect( Action& action )
  {
    for (SDL_Event event; SDL_PollEvent( &event );)
      {
        if      (event.type == SDL_QUIT)    throw Action::Quit();
        else if ((event.type == SDL_KEYDOWN) or (event.type == SDL_KEYUP))
          {
            bool active = (event.type == SDL_KEYDOWN);
            switch (event.key.keysym.sym) {
            case SDLK_RETURN: case SDLK_KP_ENTER: action.cmds.set( Action::Branch, active ); action.cmds.set( Action::Select, active ); break;
            case SDLK_SPACE:                      action.cmds.set( Action::Fire, active ); break;
            case SDLK_ESCAPE:                     action.cmds.set( Action::Jump, active ); break;
            case SDLK_RIGHT:                      action.cmds.set( Action::Right, active ); break;
            case SDLK_LEFT:                       action.cmds.set( Action::Left, active ); break;
            case SDLK_DOWN:                       action.cmds.set( Action::Down, active ); break;
            case SDLK_UP:                         action.cmds.set( Action::Up, active ); break;
            case SDLK_DELETE:                     action.cmds.set( Action::DelFwd, active ); break;
            case SDLK_BACKSPACE:                  action.cmds.set( Action::DelBwd, active ); action.cmds.set( Action::Debug, active ); break;
            case SDLK_LSHIFT: case SDLK_RSHIFT:   action.cmds.set( Action::Shift, active ); break;
            case SDLK_LALT: case SDLK_RALT:       action.cmds.set( Action::Alt, active ); break;
            default: break;
            }
          }
      }
  }

  Point<float>
  motion( Action const& action )
  {
    int hor = int(action.cmds[Action::Right])-int(action.cmds[Action::Left]);
    int ver = int(action.cmds[Action::Down])-int(action.cmds[Action::Up]);
    float scale = (hor & ver) ? M_SQRT1_2 : 1.0;
    return Point<float>( hor*scale, ver*scale );
  }

  int
  flipandwait( Screen& screen, int next_ticks )
  {
    {
      void* pixels = reinterpret_cast<void*>( &screen.pixels[0][0] );
  
      int height = Screen::height, width = Screen::width;
      Uint16 pitch = width*4;
      std::swap( pixels, sdl.scratch->pixels );
      std::swap( width, sdl.scratch->w );
      std::swap( height, sdl.scratch->h );
      std::swap( pitch, sdl.scratch->pitch );
  
      SDL_Rect offset;
      Point<int32_t>(0,0).pull( offset );
      SDL_BlitSurface( sdl.scratch, NULL, sdl.screen, &offset );
  
      std::swap( pixels, sdl.scratch->pixels );
      std::swap( width, sdl.scratch->w );
      std::swap( height, sdl.scratch->h );
      std::swap( pitch, sdl.scratch->pitch );
    }

    if (SDL_Flip( sdl.screen ) == -1)
      {
        std::cerr << "Can't flip m_screen: " << SDL_GetError() << '\n';
        throw "Unexpected SDL error";
      }
    
    int now_ticks;
    while ((now_ticks = SDL_GetTicks()) < next_ticks)
      {
        int duration = next_ticks - now_ticks;
        //std::cerr << "waiting for " << duration << "ms.\n";
        SDL_Delay( duration );
      }
    return now_ticks;
  }
}

