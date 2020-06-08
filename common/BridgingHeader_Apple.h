#ifndef __BRIDGING_HEADER_APPLE_H__
#define __BRIDGING_HEADER_APPLE_H__

#include <vector>

size_t EncodeBase64_Apple( const unsigned char * src,
   size_t srcLength,
   std::vector< unsigned char > & ref_dest );
size_t DecodeBase64_Apple( const unsigned char * src,
   size_t srcLength,
   std::vector< unsigned char > & ref_dest );
   
#endif
