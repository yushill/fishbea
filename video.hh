#ifndef __IMAGE_HH__
#define __IMAGE_HH__

#include <geometry.hh>
#include <inttypes.h>

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

template <uintptr_t WIDTH, uintptr_t HEIGHT> uintptr_t pixwidth( Pixel (&dst)[HEIGHT][WIDTH] ) { return WIDTH; }
template <uintptr_t WIDTH, uintptr_t HEIGHT> uintptr_t pixheight( Pixel (&dst)[HEIGHT][WIDTH] ) { return HEIGHT; }
template <uintptr_t WIDTH, uintptr_t HEIGHT> uintptr_t pixarea( Pixel (&dst)[HEIGHT][WIDTH] ) { return WIDTH*HEIGHT; }

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
image_fade( Pixel (&dst)[HEIGHT][WIDTH], Pixel const (&src1)[HEIGHT][WIDTH], Pixel const (&src2)[HEIGHT][WIDTH], uint8_t select )
{
  unsigned int sel1 = select + 1, sel2 = 256 - select;
  
  for (uintptr_t idx = 0; idx < HEIGHT*WIDTH; ++idx) {
    Pixel& pdst = (&dst[0][0])[idx];
    Pixel const& psrc1 = (&src1[0][0])[idx];
    Pixel const& psrc2 = (&src2[0][0])[idx];
    pdst.b = (psrc1.b*sel1 + psrc2.b*sel2) >> 8;
    pdst.g = (psrc1.g*sel1 + psrc2.g*sel2) >> 8;
    pdst.r = (psrc1.r*sel1 + psrc2.r*sel2) >> 8;
    pdst.a = 0xff;
  }
}

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

struct Screen {
  static const int32_t   width = 640;
  static const int32_t   height = 384;
  static Point<int32_t>  diag() { return Point<int32_t>( width, height ); }
  typedef Pixel          pixels_t[height][width];
  pixels_t               pixels;
  
  Screen() {}
  Screen( Screen const& _screen )
  {
    for (uintptr_t idx = 0; idx < pixarea(pixels); ++idx)
      (&pixels[0][0])[idx] = (&_screen.pixels[0][0])[idx];
  }
};
  
#endif /* __IMAGE_HH__ */
