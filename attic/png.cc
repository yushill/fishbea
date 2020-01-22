#include <iostream>
#include <fstream>
#include <zlib.h>
#include <cstdlib>
#include <cstring>
#include <cassert>
#include <inttypes.h>

template <typename T>
bool getintegral( T& seed, std::istream& _src )
{
  char buffer[sizeof (T)];
  if (not _src.read( &buffer[0], sizeof (T) )) return false;
  T value = T();
  
  int idx = 0, offset = 8*sizeof (T);
  do {
    offset -= 8;
    value |= T( uint8_t( buffer[idx++] ) ) << offset;
  } while (offset > 0);
  seed = value;
  return true;
}

struct FourCharCode
{
  char buffer[4];
  FourCharCode( uint32_t _code ) { for (int idx = 0, offset = 32; (offset -= 8) >= 0; ++idx) buffer[idx] = (_code >> offset) & 0xff; }
};
std::ostream& operator << ( std::ostream& sink, FourCharCode const& fcc )
{ sink.write( &fcc.buffer[0], sizeof (fcc.buffer) ); return sink; }

#define FOURCHARCODE( A, B, C, D )              \
  ((uint32_t(uint8_t(A)) << 24) |               \
   (uint32_t(uint8_t(B)) << 16) |               \
   (uint32_t(uint8_t(C)) <<  8) |               \
   (uint32_t(uint8_t(D)) <<  0))

struct ChunkType
{
  uint32_t code;
  ChunkType( char const* _code ) : code() { for (;*_code;++_code) code = (code << 8) | uint8_t(*_code);  }
  operator uint32_t () const { return code; }
};

int
main()
{
  char const* filename = "data/starfish.png";
  std::cout << "Opening " << filename << "\n";
  std::ifstream source( filename );
  {
    char MAGIC[] = {'\x89','P','N','G','\x0d','\x0a','\x1a','\x0a'};
    char magic[sizeof (MAGIC)];
    source.read( &magic[0], sizeof (MAGIC) );
    assert( strncmp( &magic[0], &MAGIC[0], sizeof (MAGIC) ) == 0 );
  }
  
  for (;;) {
    uint32_t chunk_size;
    assert (getintegral( chunk_size, source ));
    std::cout << "Chunk size: " << chunk_size << ".\n";
    uint32_t chunk_type;
    assert (getintegral( chunk_type, source ));
    std::cout << "Name: " << FourCharCode( chunk_type ) << "\n";
    
    {
      char chunk_data[chunk_size];
      assert (source.read( &chunk_data[0], chunk_size ));
      if      (chunk_type == ChunkType("IHDR")) {
        std::cout << "  processing header.\n";
      }
      else if (chunk_type == ChunkType("IDAT")) {
        std::cout << "  processing data.\n";
        int ret;
        z_stream strm;
        unsigned char outbuf[16384];
  
        /* allocate inflate state */
        strm.zalloc = Z_NULL;
        strm.zfree = Z_NULL;
        strm.opaque = Z_NULL;
        strm.avail_in = chunk_size;
        strm.next_in = (unsigned char*)(&chunk_data[0]);
        ret = inflateInit( &strm );
        if (ret != Z_OK) { std::cerr << "ZLIB error...\n"; throw "ZLIB error"; }
  
        /* run inflate() on input until output buffer not full */
        do {
          strm.avail_out = sizeof (outbuf);
          strm.next_out = &outbuf[0];
    
          ret = inflate( &strm, Z_NO_FLUSH );
          if (ret == Z_STREAM_ERROR) { std::cerr << "ZLIB error...\n"; throw "ZLIB error"; }
          switch (ret) {
          case Z_NEED_DICT:
            ret = Z_DATA_ERROR;     /* and fall through */
          case Z_DATA_ERROR:
          case Z_MEM_ERROR:
            (void)inflateEnd( &strm );
            std::cerr << "ZLIB error...\n";
            throw "ZLIB error";
          }

          uintptr_t have = sizeof (outbuf) - strm.avail_out;
          std::cout << "    zlib have: " << have << ".\n";
          /* TODO: export buffer */
        } while (strm.avail_out == 0);
  
        if ((strm.avail_in != 0) or (ret != Z_STREAM_END)) { std::cerr << "PNG error... corrupted IDAT compressed data\n"; throw "PNG error"; }
  
        /* clean up and return */
        (void)inflateEnd( &strm );

      }
      else if (chunk_type == ChunkType("IEND")) {
        std::cout << "  last chunk.\n";
        break;
      }
    }
    
    uint32_t crc;
    assert (getintegral( crc, source ));
  }
  
  return 0;
}
