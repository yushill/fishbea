#include <video.hh>
#include <png.h>
#include <algorithm>
#include <cassert>
#include <cstdio>

namespace { template <typename T> T failtest( T _p ) { assert( _p ); return _p; } }

void image_pngload( Pixel* _dst, uintptr_t _width, uintptr_t _height, char const* _filepath )
{
  FILE* fp = failtest( fopen( _filepath, "rb" ) );
  {
    /* Checking if file exists and is a PNG file. */
    png_byte header[8];
    failtest( (fread( header, 1, 8, fp ) == 8) and (png_sig_cmp(header, 0, 8) == 0) );
  }
  
  png_structp png_ptr = failtest( png_create_read_struct( PNG_LIBPNG_VER_STRING, NULL, NULL, NULL ) );
  png_set_sig_bytes( png_ptr, 8 ); /* skipping 8 signatures bytes */
  png_infop info_ptr = failtest( png_create_info_struct( png_ptr ) );

  /* setjmp libpng exceptions */
  failtest( not setjmp( png_jmpbuf( png_ptr ) ) );
  

  png_init_io( png_ptr, fp ); /* using fp as input */

  /* pnglib will, by default, allocate memory for the image data,
     and store the image data row by row, where each row is a 1-dimensional
     array of bytes, and return an array of pointers to all the rows.

     For our uses, we want the rows to be stored consecutively in memory,
     forming a two-dimensional array.  pnglib does not guarantee this,
     so we allocate the memory, and set the row pointers to point to
     the start of each row, and give this info to pnglib through the png_ptr
     structure.
  */

  /* Read the header of the PNG file, to get the height and width. */
  png_read_info( png_ptr, info_ptr );

  failtest( png_get_image_height( png_ptr, info_ptr ) == _height );
  failtest( png_get_image_width( png_ptr, info_ptr ) == _width );
  failtest( png_get_bit_depth( png_ptr, info_ptr ) == 8 );
  failtest( png_get_channels( png_ptr, info_ptr ) == 4 );
  failtest( png_get_color_type(png_ptr, info_ptr) == PNG_COLOR_TYPE_RGBA );
  
  png_bytep* row_pointers = failtest( new png_bytep[_height] );
  for (uintptr_t row = 0; row < _height; ++row)
    row_pointers[row] = (png_byte*)(&_dst[row*_width]);

  png_set_rows( png_ptr, info_ptr, row_pointers );

  /* Read the image data from the PNG file */
  png_read_image( png_ptr, row_pointers );
  png_read_end( png_ptr, info_ptr );
  
  /* PNG is RGBA wheras we prefer BGRA */
  for (uintptr_t idx = 0; idx < _width*_height; ++idx) std::swap(_dst[idx].b, _dst[idx].r);
}
