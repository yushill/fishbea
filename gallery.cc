#include <gallery.hh>
#include <video.hh>
#include <iostream>

Pixel gallery::hero[36][48];
Pixel gallery::gray_ghost[36][48];
Pixel gallery::blue_ghost[36][48];
Pixel gallery::red_ghost[36][48];
Pixel gallery::shell[48][48];
Pixel gallery::shiny_shell[48][48];
Pixel gallery::starfish[32][48];
Pixel gallery::shiny_starfish[32][48];
Screen::pixels_t gallery::classic_bg;

ImageStore gallery::__is__( gallery::__init__, gallery::__exit__ );

void gallery::__init__()
{
  image_pngload( hero, "data/Nemo.png" );
  
  image_apply( Ghostify(), gray_ghost, hero );
  image_apply( Blueify(), blue_ghost, hero );
  image_apply( Redify(), red_ghost, hero );
  
  image_pngload( shell, "data/door.png" );
  
  image_apply( Hilite(), shiny_shell, shell );
  
  image_pngload( starfish, "data/starfish.png" );
  
  image_apply( Hilite(), shiny_starfish, starfish );
  
  image_pngload( classic_bg, "data/background.png" );
}

void gallery::__exit__()
{}
