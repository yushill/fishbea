#ifndef __GALLERY_HH__
#define __GALLERY_HH__

#include <video.hh>

struct gallery
{
  static Pixel hero[36][48];
  static Pixel gray_ghost[36][48];
  static Pixel blue_ghost[36][48];
  static Pixel red_ghost[36][48];
  static Pixel shell[48][48];
  static Pixel shiny_shell[48][48];
  static Pixel starfish[32][48];
  static Pixel shiny_starfish[32][48];
  static screen_t classic_bg;
  
  static void __init__( SDL_Surface* _screen );
  static void __exit__();
  
  static ImageStore __is__;
};

#endif /* __GALLERY_HH__ */
