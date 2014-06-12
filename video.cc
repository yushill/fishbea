#include <video.hh>
#include <SDL/SDL.h>
#include <SDL/SDL_image.h>
#include <iostream>
#include <stdexcept>

VideoConfig::VideoConfig()
  : screen()
{
  // SDL environment setup
  if (SDL_Init( SDL_INIT_EVERYTHING ) == -1)
    throw std::runtime_error( std::string( "Can't init SDL: " ) + SDL_GetError() + '\n' );
  
  SDL_WM_SetCaption( "FishBea", NULL );
  
  screen = SDL_SetVideoMode( width, height, 16, SDL_SWSURFACE );
  if (not screen)
    throw std::runtime_error( std::string( "Can't set video mode: " ) + SDL_GetError() + "\n" );
    
  ImageStore::pool->init();
}
  
VideoConfig::~VideoConfig()
{
  ImageStore::pool->exit();
  SDL_Quit();
}

SDL_Surface*
load_image( std::string filename )
{
  SDL_Surface* img;
  
  if ((img = IMG_Load( filename.c_str() )))
    {
      SDL_Surface* source = img;
      img = SDL_DisplayFormatAlpha( source );
      SDL_FreeSurface( source );
    }
  
  if (not img)
    {
      std::cerr << "Can't open '" << filename << "'.\n";
      throw "Unexpected SDL error";
    }
  
  return img;
}

ImageStore* ImageStore::pool = 0;
