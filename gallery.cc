#include <gallery.hh>
#include <video.hh>
#include <action.hh>

Pixel gallery::hero[36][48];
Pixel gallery::gray_ghost[36][48];
Pixel gallery::blue_ghost[36][48];
Pixel gallery::red_ghost[36][48];
Pixel gallery::shell[48][48];
Pixel gallery::shiny_shell[48][48];
Pixel gallery::starfish[32][48];
Pixel gallery::shiny_starfish[32][48];
Screen::pixels_t gallery::classic_bg;

namespace {
  struct Store : public Action::Store
  {
    void init()
    {
      image_pngload( gallery::hero, "data/Nemo.png" );
  
      image_apply( Ghostify(), gallery::gray_ghost, gallery::hero );
      image_apply( Blueify(), gallery::blue_ghost, gallery::hero );
      image_apply( Redify(), gallery::red_ghost, gallery::hero );
  
      image_pngload( gallery::shell, "data/door.png" );
  
      image_apply( Hilite(), gallery::shiny_shell, gallery::shell );
  
      image_pngload( gallery::starfish, "data/starfish.png" );
  
      image_apply( Hilite(), gallery::shiny_starfish, gallery::starfish );
  
      image_pngload( gallery::classic_bg, "data/background.png" );
    }
    
    void exit() {}
  } __as__;
}
