#ifndef SmoothFactory_hpp
#define SmoothFactory_hpp

#include <string>
#include <memory>
#include <map>
#include "Smooth.hpp"

// Implemented smoothing types
enum SMOOTH_TYPE
{
   SMOOTH_TYPE_NONE,
   SMOOTH_TYPE_LPF_IPP,   
   // New smooth types go here
   
   SMOOTH_TYPE_UNKNOWN
};

// Factory for filtering motion.
// See @Smooth.hpp for more information.
class SmoothFactory
{
public:   
   static SMOOTH_TYPE SmoothType( std::string type );
   static std::unique_ptr< Smooth > Create( SMOOTH_TYPE type );
};

#endif
