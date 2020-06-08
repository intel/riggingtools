#include <sstream>
#include <unordered_map>
#include "SmoothFactory.hpp"

// Built-in smooth classes
#include "Smooth_lpfIpp.hpp"

Smooth::SMOOTH_TYPE SmoothFactory::SmoothType( std::string type )
{
   static std::unordered_map< std::string, Smooth::SMOOTH_TYPE > map = {
      { "none",      Smooth::SMOOTH_TYPE_NONE },
      { "lpf_ipp",   Smooth::SMOOTH_TYPE_LPF_IPP }
   };
   
   auto it = map.find( type );
   if ( it == map.end() )
      return Smooth::SMOOTH_TYPE_NONE;
   else
      return (*it).second;
}
std::unique_ptr< Smooth > SmoothFactory::Create( Smooth::SMOOTH_TYPE type )
{
   // Create a new importer
   switch ( type )
   {
      case Smooth::SMOOTH_TYPE_LPF_IPP: return std::unique_ptr< Smooth >( new Smooth_lpfIpp() );
      default: return std::unique_ptr< Smooth >( nullptr );
   }
}
