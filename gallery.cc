#include <gallery.hh>
#include <video.hh>
#include <SDL/SDL_video.h>
#include <iostream>

SDL_Surface* gallery::hero = 0;
SDL_Surface* gallery::gray_ghost = 0;
SDL_Surface* gallery::blue_ghost = 0;
SDL_Surface* gallery::red_ghost = 0;
SDL_Surface* gallery::shell = 0;
SDL_Surface* gallery::shiny_shell = 0;
SDL_Surface* gallery::classic_bg = 0;

ImageStore gallery::__is__( gallery::__init__, gallery::__exit__ );

void gallery::__init__( SDL_Surface* _screen )
{
  hero = load_image( "data/Nemo.png" );
  gray_ghost = SDL_ConvertSurface( hero, hero->format, hero->flags );
  blue_ghost = SDL_ConvertSurface( hero, hero->format, hero->flags );
  red_ghost = SDL_ConvertSurface( hero, hero->format, hero->flags );
  image_apply( Ghostify(), gray_ghost );
  image_apply( Blueify(), blue_ghost );
  image_apply( Redify(), red_ghost );
  shell = load_image( "data/door.png" );
  shiny_shell =  SDL_ConvertSurface( shell, shell->format, shell->flags );
  classic_bg = load_image( "data/background.png" );
  image_apply( Hilite(), shiny_shell );
  std::cerr << "Been here.\n";
}

void gallery::__exit__()
{
  SDL_Surface* surfaces[] = {
    hero,
    gray_ghost,
    blue_ghost,
    red_ghost,
    shell,
    shiny_shell,
    classic_bg
  };
  for (SDL_Surface** s = &surfaces[((sizeof surfaces) / (sizeof surfaces[0]))]; --s >= &surfaces[0];) {
    if (*s) SDL_FreeSurface( *s );
  }
}
