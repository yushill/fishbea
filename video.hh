#ifndef __IMAGE_HH__
#define __IMAGE_HH__

#include <geometry.hh>
#include <string>
#include <inttypes.h>

struct SDL_Surface;

struct VideoConfig
{
  static const int width = 640;
  static const int height = 384;
  
  VideoConfig();
  ~VideoConfig();
  SDL_Surface* screen;
};

extern SDL_Surface* load_image( std::string filename );

struct ImageStore {
  typedef void (*init_method_t)( SDL_Surface* _screen );
  typedef void (*exit_method_t)();
  init_method_t init_method;
  exit_method_t exit_method;
  ImageStore* next;
  static ImageStore* pool;
  ImageStore( init_method_t _init, exit_method_t _exit )
    : init_method( _init ), exit_method( _exit ), next( pool ) { pool = this; }
  void init( SDL_Surface* _screen ) { if (next) next->init( _screen ); init_method( _screen ); }
  void exit() { exit_method(); if (next) next->exit(); }
};

struct Hilite { void operator() ( uint8_t* imgp ) const {
  imgp[0] = (imgp[0] >> 1) + 0x7f;
  imgp[1] = (imgp[1] >> 1) + 0x7f;
  imgp[2] = (imgp[2] >> 1) + 0x7f;
}};

struct Grayify { void operator() ( uint8_t* imgp ) const {
  uint8_t gray = (0x4c8b43*imgp[0] + 0x9645a2*imgp[1] + 0x1d2f1b*imgp[2])>>24;
  imgp[0] = gray;
  imgp[1] = gray;
  imgp[2] = gray;
}};

struct Ghostify { void operator() ( uint8_t* imgp ) const {
  uint8_t gray = (0x4c8b43*imgp[0] + 0x9645a2*imgp[1] + 0x1d2f1b*imgp[2])>>24;
  imgp[0] = gray;
  imgp[1] = gray;
  imgp[2] = gray;
  imgp[3] /= 2;
}};

struct Blueify { void operator() ( uint8_t* imgp ) const {
  uint8_t gray = (0x4c8b43*imgp[0] + 0x9645a2*imgp[1] + 0x1d2f1b*imgp[2])>>24;
  imgp[0] = 0;
  imgp[1] = 0;
  imgp[2] = gray;
  imgp[3] /= 2;
}};

struct Redify { void operator() ( uint8_t* imgp ) const {
  uint8_t gray = (0x4c8b43*imgp[0] + 0x9645a2*imgp[1] + 0x1d2f1b*imgp[2])>>24;
  imgp[0] = gray;
  imgp[1] = 0;
  imgp[2] = 0;
  imgp[3] /= 2;
}};

template <uint8_t R, uint8_t G, uint8_t B, uint8_t A>
struct Fill { void operator() ( uint8_t* imgp ) const {
  imgp[0] = R; imgp[1] = G; imgp[2] = B; imgp[3] = A;
}};

struct Fade {
  uintptr_t const boffset, eoffset;
  typedef unsigned int ui;
  ui const x;
  static ui const prec = 0x10000;
  
  typedef uint8_t* ptr;
  template <typename imgT>
  Fade( imgT* dst, imgT* beg, imgT* end, double _x )
    : boffset( ptr(beg->pixels) - ptr(dst->pixels) ),
      eoffset( ptr(end->pixels) - ptr(dst->pixels) ),
      x( _x*prec )
  {}
  void operator() ( uint8_t* pxs ) const
  {
    pxs[0] = (pxs[eoffset+0]*x+pxs[boffset+0]*(prec-x))/prec;
    pxs[1] = (pxs[eoffset+1]*x+pxs[boffset+1]*(prec-x))/prec;
    pxs[2] = (pxs[eoffset+2]*x+pxs[boffset+2]*(prec-x))/prec;
  }
};

template <typename kerT, typename imgT>
imgT*
image_apply( kerT const& ker, imgT* img )
{
  for (uint8_t *imgp = (uint8_t*)img->pixels, *end = &imgp[(img->w*img->h)*4]; imgp < end; imgp += 4)
    ker( imgp );
  return img;
}

struct Pixel {
  uint8_t r,g,b,a;
  Pixel() {}
  Pixel( uint8_t _r, uint8_t _g, uint8_t _b, uint8_t _a ) : r(_r),g(_g),b(_b),a(_a) {}
  void set( uint8_t _r, uint8_t _g, uint8_t _b, uint8_t _a ) { new (this) Pixel( _r,_g,_b,_a ); }
};

#endif /* __IMAGE_HH__ */
