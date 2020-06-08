#import "BridgingHeader_Apple.h"
#import <Foundation/NSData.h>
#include <string>
#include <vector>

size_t EncodeBase64_Apple( const unsigned char * src,
   size_t srcLength,
   std::vector< unsigned char > & ref_dest )
{
   size_t returnValue = 0;
   
   NSData * nsSrc = [NSData dataWithBytes:src length:srcLength];
   NSData * nsDest = [nsSrc base64EncodedDataWithOptions: 0];
   
   // Data needs to be copied
   returnValue = size_t([nsDest length]);
   memcpy( &ref_dest[0],
      [nsDest bytes],
      returnValue );

   return returnValue;
}
size_t DecodeBase64_Apple( const unsigned char * src,
   size_t srcLength,
   std::vector< unsigned char > & ref_dest )
{
   size_t returnValue = 0;
   
   NSData * nsSrc = [NSData dataWithBytes:src length:srcLength];
   NSData * nsDest = [[NSData alloc] initWithBase64EncodedData:nsSrc options:NSDataBase64DecodingIgnoreUnknownCharacters];
   
   // Data needs to be copied
   returnValue = size_t([nsDest length]);
   memcpy( &ref_dest[0],
      [nsDest bytes],
      returnValue );

   return returnValue;
}
