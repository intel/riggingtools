#define CATCH_CONFIG_RUNNER
#include <catch2/catch.hpp>

#include "Utility.hpp"

using namespace Catch::clara;

int main( int argc, char* argv[] )
{
   Catch::Session session;

   auto cli = session.cli()
      | Opt( g_url, "url")
         ["-u"]["--url"]
         ("**REQUIRED** URL to json file on local disk" );

   session.cli( cli );

   auto ret = session.applyCommandLine( argc, argv );
   if ( ret )
   {
      return ret;
   }
   
   if ( g_url == "" )
   {
      printf( "  URL to a json file is required (use --help for more info)\n" );
      return 0;
   }

   return session.run();
}
