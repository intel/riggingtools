#include "Compression.hpp"
#include <zfp.h>
#include <stdexcept>
#include <cstring>

#if defined(_USE_WIN32_BASE64_)
   #include <Windows.h>
   #include <Wincrypt.h>
#elif defined(_USE_APPLE_NS_BASE64_)
   #include "BridgingHeader_Apple.h"
#elif defined(_USE_GLIB_BASE64_)
   #include <glib.h>
#elif defined(_USE_APR_BASE64_)
   #include <apr_base64.h>
#endif
namespace Compression
{
   void DecodeZfp( const uint8_t * compressedData,
      size_t dataSize,
      std::vector<double> & array )
   {
      // Create a stream
      zfp_stream * zfp = zfp_stream_open( NULL );
      
      // Associate bit stream with allocated buffer
      bitstream * stream = stream_open( (char *)compressedData, dataSize );
      zfp_stream_set_bit_stream( zfp, stream );
      
      // Read the header
      zfp_field * field = zfp_field_alloc();
      zfp_read_header( zfp, field, ZFP_HEADER_FULL );
      
      // Allocate output
      array.insert( std::begin( array ), field->nx, 0.0 );
      zfp_field_set_pointer( field, &array[0] );
      
      // Compress array and output compressed stream
      if (!zfp_decompress( zfp, field ))
      {
         throw std::runtime_error( "zfp decompression failed" );
      }

      // Clean up
      zfp_field_free( field );
      zfp_stream_close( zfp );
      stream_close( stream );
   }
   void EncodeZfp( double * array,
      size_t arrayDimension,
      void * buffer,
      size_t & bufferSize )
   {
      // Allocate meta data for the array
      zfp_field * field = zfp_field_1d( array,
         zfp_type_double,
         (uint)arrayDimension );

      // Create a stream
      zfp_stream * zfp = zfp_stream_open( NULL );

      // Set accuracy tolerance
      zfp_stream_set_accuracy( zfp, 1e-4 ) ;

      // Associate bit stream with allocated buffer
      bitstream * stream = stream_open( buffer, bufferSize );
      zfp_stream_set_bit_stream( zfp, stream );
      
      // Write the header
      zfp_write_header( zfp, field, ZFP_HEADER_FULL );

      // Compress array and output compressed stream
      bufferSize = zfp_compress( zfp, field );
      if (!bufferSize)
      {
         throw std::runtime_error( "zfp compression failed" );
      }

      // Clean up
      zfp_field_free( field );
      zfp_stream_close( zfp );
      stream_close( stream );
   }

   // This will resize @ref_dest if needed
   size_t EncodeBase64( const unsigned char * src,
      size_t srcLength,
      std::vector< unsigned char > & ref_dest )
   {
      size_t returnValue = 0;
      
#if defined(_USE_WIN32_BASE64_)
      const DWORD flags = CRYPT_STRING_BASE64 | CRYPT_STRING_NOCRLF;
      DWORD requiredLength;
      BOOL success = CryptBinaryToStringA( src,
         DWORD(srcLength),
         flags,
         NULL,
         &requiredLength );
      if ( success )
      {
         if ( ref_dest.size() < size_t(requiredLength) )
            ref_dest.resize( size_t(requiredLength) );
         success = CryptBinaryToStringA( src,
            DWORD( srcLength ),
            flags,
            (LPSTR)&ref_dest[0],
            &requiredLength );
         if ( success )
            returnValue = size_t( requiredLength );
      }
#elif defined(_USE_APPLE_NS_BASE64_)
      size_t requiredLength = srcLength * 3; // Assume encoded size is greater-than
      if ( ref_dest.size() < requiredLength )
         ref_dest.resize( requiredLength );
      returnValue = EncodeBase64_Apple( src,
         srcLength,
         ref_dest );
#elif defined(_USE_GLIB_BASE64_)
      size_t requiredLength = srcLength * 3; // Assume encoded size is greater-than
      if ( ref_dest.size() < size_t(requiredLength) )
         ref_dest.resize( requiredLength );
      char * base64 = g_base64_encode( src,
         srcLength );
      if ( base64 )
      {
         returnValue = strlen( base64 );
         memcpy( &ref_dest[0],
            base64,
            returnValue );
      }
#elif defined(_USE_APR_BASE64_)
      int requiredLength = apr_base64_encode_len( srcLength );
      if ( ref_dest.size() < size_t(requiredLength) )
         ref_dest.resize( requiredLength );
      returnValue = apr_base64_encode( reinterpret_cast< char* >(&ref_dest[0]),
         reinterpret_cast< const char *>(src),
         int(srcLength) );
#else
      #error No base64 implementation
      (void)src;
      (void)srcLength;
      (void)ref_dest;
#endif
         
      return returnValue;
   }
   // This will resize @ref_dest if needed
   size_t DecodeBase64( const unsigned char * src,
      size_t srcLength,
      std::vector< unsigned char > & ref_dest )
   {
      size_t returnValue = 0;
      (void)srcLength;
      
#if defined(_USE_WIN32_BASE64_)
      DWORD requiredLength;
      BOOL success = CryptStringToBinaryA( (LPSTR)src,
         DWORD( srcLength ),
         CRYPT_STRING_BASE64,
         NULL,
         &requiredLength,
         NULL,
         NULL );
      if ( success )
      {
         if ( ref_dest.size() < size_t( requiredLength ) )
            ref_dest.resize( size_t( requiredLength ) );
         success = CryptStringToBinaryA( (LPSTR)src,
            DWORD( srcLength ),
            CRYPT_STRING_BASE64,
            &ref_dest[0],
            &requiredLength,
            NULL,
            NULL );
         if ( success )
            returnValue = size_t( requiredLength );
      }
#elif defined(_USE_APPLE_NS_BASE64_)
      size_t requiredLength = srcLength; // Assume decoded size is equal or less-than
      if ( ref_dest.size() < requiredLength )
         ref_dest.resize( requiredLength );
      returnValue = DecodeBase64_Apple( src,
         requiredLength,
         ref_dest );
#elif defined(_USE_GLIB_BASE64_)
      size_t requiredLength = srcLength; // Assume decoded size is equal or less-than 
      if ( ref_dest.size() < requiredLength )
         ref_dest.resize( requiredLength );
      unsigned char * bytes = g_base64_decode( reinterpret_cast< const char *>(src),
         &requiredLength );
      if ( bytes )
      {
         returnValue = requiredLength;
         memcpy( &ref_dest[0],
            bytes,
            returnValue );
      }
#elif defined(_USE_APR_BASE64_)
      size_t requiredLength = apr_base64_decode_len( reinterpret_cast< const char *>(src) );
      if ( ref_dest.size() < requiredLength )
         ref_dest.resize( requiredLength );
     returnValue = apr_base64_decode( reinterpret_cast< char* >(&ref_dest[0]),
         reinterpret_cast< const char *>(src) );
#else
      #error No base64 implementation
      (void)src;
      (void)srcLength;
      (void)ref_dest;
#endif
      
      return returnValue;
   }
}
