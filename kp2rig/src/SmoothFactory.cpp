#include <sstream>
#include <unordered_map>
#include "SmoothFactory.hpp"

// Include implemented smooth classes here
#include "Smooth_lpfIpp.hpp"

SMOOTH_TYPE SmoothFactory::SmoothType( std::string type )
{
   static std::unordered_map< std::string, SMOOTH_TYPE > map = {
      { "none",      SMOOTH_TYPE_NONE },
      { "lpf_ipp",   SMOOTH_TYPE_LPF_IPP }
   };

   auto it = map.find( type );
   if ( it == map.end() )
      return SMOOTH_TYPE_NONE;
   else
      return (*it).second;
}
std::string SmoothFactory::SmoothType( SMOOTH_TYPE type )
{
   static std::unordered_map< SMOOTH_TYPE, std::string > map = {
      { SMOOTH_TYPE_NONE,       "none"      },
      { SMOOTH_TYPE_LPF_IPP,    "lpf_ipp"   }
   };

   auto it = map.find( type );
   if ( it == map.end() )
      return "none";
   else
      return (*it).second;
}
std::unique_ptr< Smooth > SmoothFactory::Create( SMOOTH_TYPE type )
{
   // Create a new importer
   switch ( type )
   {
#ifdef HAVE_IPP
      case SMOOTH_TYPE_LPF_IPP: return std::unique_ptr< Smooth >( new Smooth_lpfIpp() );
#endif
      default: return std::unique_ptr< Smooth >( nullptr );
   }
}
