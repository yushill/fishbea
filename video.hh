#ifndef __IMAGE_HH__
#define __IMAGE_HH__

#include <geometry.hh>
#include <string>
#include <inttypes.h>

struct SDL_Surface;

struct VideoConfig
{
  static const int32_t width = 640;
  static const int32_t height = 384;
  static Point<int32_t> diag() { return Point<int32_t>( width, height ); }
  
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

struct Pixel
{
  uint8_t b,g,r,a;
  Pixel() {}
  Pixel( uint8_t _b, uint8_t _g, uint8_t _r, uint8_t _a ) : b(_b),g(_g),r(_r),a(_a) {}
  void set( uint8_t _b, uint8_t _g, uint8_t _r, uint8_t _a ) { new (this) Pixel( _b,_g,_r,_a ); }
};

template <uintptr_t WIDTH, uintptr_t HEIGHT>
void pixcpy( Pixel (&dst)[HEIGHT][WIDTH], Pixel (&src)[HEIGHT][WIDTH] )
{
  for (int idx = 0; idx < WIDTH*HEIGHT; ++idx) (&dst[0][0])[idx] = (&src[0][0])[idx];
}

typedef Pixel screen_t[384][640];

template <uintptr_t WIDTH, uintptr_t HEIGHT> uintptr_t pixwidth( Pixel (&dst)[HEIGHT][WIDTH] ) { return WIDTH; }
template <uintptr_t WIDTH, uintptr_t HEIGHT> uintptr_t pixheight( Pixel (&dst)[HEIGHT][WIDTH] ) { return HEIGHT; }

template <typename kerT, uintptr_t WIDTH, uintptr_t HEIGHT>
void
image_apply( kerT const& ker, Pixel (&dst)[HEIGHT][WIDTH] )
{
  for (uintptr_t idx = 0; idx < HEIGHT*WIDTH; ++idx)
    ker( (&dst[0][0])[idx] );
}

template <typename kerT, uintptr_t WIDTH, uintptr_t HEIGHT>
void
image_apply( kerT const& ker, Pixel (&dst)[HEIGHT][WIDTH], Pixel (&src)[HEIGHT][WIDTH] )
{
  for (uintptr_t idx = 0; idx < HEIGHT*WIDTH; ++idx)
    { (&dst[0][0])[idx] = (&src[0][0])[idx]; ker( (&dst[0][0])[idx] ); }
}

extern void image_pngload( Pixel* _dst, uintptr_t _width, uintptr_t _height, char const* _filepath );

template <uintptr_t WIDTH, uintptr_t HEIGHT>
void
image_pngload( Pixel (&dst)[HEIGHT][WIDTH], char const* _filepath )
{
  image_pngload( &dst[0][0], WIDTH, HEIGHT, _filepath );
}

template <uintptr_t WIDTH, uintptr_t HEIGHT>
void
image_fade( Pixel (&dst)[HEIGHT][WIDTH], Pixel (&src1)[HEIGHT][WIDTH], Pixel (&src2)[HEIGHT][WIDTH], uint8_t select )
{
  unsigned int c2 = select + 1, c1 = 256 - select;
  
  for (uintptr_t idx = 0; idx < HEIGHT*WIDTH; ++idx) {
    Pixel& pdst = (&dst[0][0])[idx];
    Pixel& psrc1 = (&dst[0][0])[idx];
    Pixel& psrc2 = (&dst[0][0])[idx];
    pdst.b = psrc1.b*c1 + psrc2.b*c2;
    pdst.g = psrc1.g*c1 + psrc2.g*c2;
    pdst.r = psrc1.r*c1 + psrc2.r*c2;
  }
}

struct Thumb
{
  screen_t screen;
  Thumb( screen_t _screen )
  {
    for (uintptr_t idx = 0; idx < pixwidth(screen)*pixheight(screen); ++idx)
      (&screen[0][0])[idx] = (&_screen[0][0])[idx];
  }
};

struct Hilite { void operator() ( Pixel& pix ) const {
  pix.b = (pix.b >> 1) + 0x7f;
  pix.g = (pix.g >> 1) + 0x7f;
  pix.r = (pix.r >> 1) + 0x7f;
}};

struct Grayify { void operator() ( Pixel& pix ) const {
  uint8_t gray = (0x4c8b43*pix.b + 0x9645a2*pix.g + 0x1d2f1b*pix.r)>>24;
  pix.b = gray;
  pix.g = gray;
  pix.r = gray;
}};

struct Ghostify { void operator() ( Pixel& pix ) const {
  uint8_t gray = (0x4c8b43*pix.b + 0x9645a2*pix.g + 0x1d2f1b*pix.r)>>24;
  pix.b = gray;
  pix.g = gray;
  pix.r = gray;
  pix.a /= 2;
}};

struct Blueify { void operator() ( Pixel& pix ) const {
  uint8_t gray = (0x4c8b43*pix.b + 0x9645a2*pix.g + 0x1d2f1b*pix.r)>>24;
  pix.b = gray;
  pix.g = 0x80;
  pix.r = 0x80;
  pix.a /= 2;
}};

struct Redify { void operator() ( Pixel& pix ) const {
  uint8_t gray = (0x4c8b43*pix.b + 0x9645a2*pix.g + 0x1d2f1b*pix.r)>>24;
  pix.b = 0x80;
  pix.g = 0x80;
  pix.r = gray;
  pix.a /= 2;
}};

template <uint8_t B, uint8_t G, uint8_t R, uint8_t A>
struct Fill { void operator() ( Pixel& pix ) const { pix.b = B; pix.g = G; pix.r = R; pix.a = A; }};

// struct Fade
// {
//   uintptr_t const boffset, eoffset;
//   typedef unsigned int ui;
//   ui const x;
//   static ui const prec = 0x10000;
  
//   typedef uint8_t* ptr;
//   template <typename imgT>
//   Fade( imgT* dst, imgT* beg, imgT* end, double _x )
//     : boffset( ptr(beg->pixels) - ptr(dst->pixels) ),
//       eoffset( ptr(end->pixels) - ptr(dst->pixels) ),
//       x( _x*prec )
//   {}
//   void operator() ( uint8_t* pxs ) const
//   {
//     pxs[0] = (pxs[eoffset+0]*x+pxs[boffset+0]*(prec-x))/prec;
//     pxs[1] = (pxs[eoffset+1]*x+pxs[boffset+1]*(prec-x))/prec;
//     pxs[2] = (pxs[eoffset+2]*x+pxs[boffset+2]*(prec-x))/prec;
//   }
// };


#endif /* __IMAGE_HH__ */
