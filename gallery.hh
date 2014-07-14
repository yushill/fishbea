#ifndef __GALLERY_HH__
#define __GALLERY_HH__

#include <video.hh>

struct gallery
{
  static SDL_Surface* hero;
  static SDL_Surface* gray_ghost;
  static SDL_Surface* blue_ghost;
  static SDL_Surface* red_ghost;
  static SDL_Surface* shell;
  static SDL_Surface* shiny_shell;
  static SDL_Surface* starfish;
  static SDL_Surface* shiny_starfish;
  static SDL_Surface* classic_bg;
  
  static void __init__( SDL_Surface* _screen );
  static void __exit__();
  
  static ImageStore __is__;
};

#endif /* __GALLERY_HH__ */
