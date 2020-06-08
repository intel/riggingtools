#ifndef Compression_h
#define Compression_h

#include <cstdint>
#include <stddef.h>
#include <vector>

namespace Compression
{
   void DecodeZfp( const uint8_t * compressedData,
      size_t dataSize,
      std::vector<double> & array );
   void EncodeZfp( double * array,
      size_t arrayDimension,
      void * buffer,
      size_t & bufferSize );
   size_t EncodeBase64( const unsigned char * src,
      size_t srcLength,
      std::vector< unsigned char > & ref_dest );
   size_t DecodeBase64( const unsigned char * src,
      size_t srcLength,
      std::vector< unsigned char > & ref_dest );
}
#endif
