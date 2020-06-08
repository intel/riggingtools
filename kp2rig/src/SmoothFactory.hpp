#ifndef SmoothFactory_hpp
#define SmoothFactory_hpp

#include <string>
#include <memory>
#include <map>
#include "Smooth.hpp"

class SmoothFactory
{
public:
   static Smooth::SMOOTH_TYPE SmoothType( std::string type );
   static std::unique_ptr< Smooth > Create( Smooth::SMOOTH_TYPE type );
};

#endif
