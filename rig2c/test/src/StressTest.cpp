#include <catch2/catch.hpp>

#include <thread>
#include <chrono>
#include <iostream>
#include <random>
#include <set>
#include <functional>
#include <vector>
#include <unordered_map>
#include "rig2cDelegates.h"
#include "Utility.hpp"
#include "Callbacks.hpp"

#ifdef _WIN32
   #include <direct.h>
   #define getcwd _getcwd
#else
   #include <unistd.h>
#endif

TEST_CASE( "basic", "[basic]" )
{
	SECTION( "dynamic_linking" )
	{
      printf( "Current dir: %s\n", getcwd( NULL, 0 ) );
      
      Utility * utility = Utility::GetInstance();
      
      REQUIRE_NOTHROW( utility->LoadLib() );

      utility->CloseLib();
   }
   
   SECTION( "function_signatures" )
   {
      Utility * utility = Utility::GetInstance();
      
      REQUIRE_NOTHROW( utility->LoadLib() );
      
      for ( auto functionPair : utility->GetFunctions() )
      {
         CHECK( functionPair.second != nullptr );
      }
      
      utility->CloseLib();
   }
   
   SECTION( "no_initialize" )
   {
      const std::string jsonFilename = g_url;
      
      Utility * utility = Utility::GetInstance();
      
      REQUIRE_NOTHROW( utility->LoadLib() );
      
      rig_RETURN_CODE returnValue = rig_NO_ERROR;
      
      // Read should fail
      returnValue = (rig_readDelegate(utility->GetFunctions()[ "rig_read" ]))( jsonFilename.c_str() );
      CHECK( returnValue == rig_API_NOT_INITIALIZED );
       
      utility->CloseLib();
   }
   
   SECTION( "no_callbacks" )
   {
      const std::string jsonFilename = g_url;
      
      Utility * utility = Utility::GetInstance();
      
      REQUIRE_NOTHROW( utility->LoadLib() );
      
      rig_RETURN_CODE returnValue = rig_NO_ERROR;
      
      // Initialize the lib
      returnValue = (rig_initializeDelegate(utility->GetFunctions()[ "rig_initialize" ]))( nullptr );
      CHECK( returnValue == rig_NO_ERROR );
      
      // Read should fail
      returnValue = (rig_readDelegate(utility->GetFunctions()[ "rig_read" ]))( jsonFilename.c_str() );
      CHECK( returnValue == rig_NO_CALLBACK );
      
      utility->CloseLib();
   }
   
   SECTION( "bad_url" )
   {
      const std::string jsonFilename = "blf91msifr8234ah.json";
      
      Utility * utility = Utility::GetInstance();
      
      REQUIRE_NOTHROW( utility->LoadLib() );
      
      rig_RETURN_CODE returnValue = rig_NO_ERROR;
      
      // Initialize the lib
      returnValue = (rig_initializeDelegate(utility->GetFunctions()[ "rig_initialize" ]))( nullptr );
      CHECK( returnValue == rig_NO_ERROR );
      
      // We need at least one callback
      (rig_setBoundsCallbackDelegate(utility->GetFunctions()[ "rig_setBoundsCallback" ]))( [](auto rigId, auto startTime, auto endTime )
      { (void)rigId; (void)startTime; (void)endTime; } );
      
      // Read should fail
      returnValue = (rig_readDelegate(utility->GetFunctions()[ "rig_read" ]))( jsonFilename.c_str() );
      CHECK( returnValue == rig_BAD_PATH );
      
      utility->CloseLib();
   }
   
   SECTION( "url" )
   {
      const std::string jsonFilename = g_url;
      
      Utility * utility = Utility::GetInstance();
      
      REQUIRE_NOTHROW( utility->LoadLib() );
      
      rig_RETURN_CODE returnValue = rig_NO_ERROR;
      
      // Initialize the lib
      returnValue = (rig_initializeDelegate(utility->GetFunctions()[ "rig_initialize" ]))( nullptr );
      CHECK( returnValue == rig_NO_ERROR );
      
      // We need at least one callback
      (rig_setBoundsCallbackDelegate(utility->GetFunctions()[ "rig_setBoundsCallback" ]))( [](auto rigId, auto startTime, auto endTime )
      { (void)rigId; (void)startTime; (void)endTime; } );
      
      // Read should succeed
      returnValue = (rig_readDelegate(utility->GetFunctions()[ "rig_read" ]))( jsonFilename.c_str() );
      INFO((std::string("Make sure ") + jsonFilename.c_str() + std::string(" exists, is a valid rig JSON file, and was created with a recent version of kp2rig")).c_str())
      REQUIRE( returnValue == rig_NO_ERROR );
      
      utility->CloseLib();
   }
   
   SECTION( "only_bounds_callback" )
   {
      const std::string jsonFilename = g_url;
      
      Utility * utility = Utility::GetInstance();
      
      REQUIRE_NOTHROW( utility->LoadLib() );
      
      rig_RETURN_CODE returnValue = rig_NO_ERROR;
      
      // Initialize the lib
      returnValue = (rig_initializeDelegate(utility->GetFunctions()[ "rig_initialize" ]))( nullptr );
      CHECK( returnValue == rig_NO_ERROR );
      
      static int numBoundCallbacks;
      OnBoundsDelegate boundsCallback = [](auto rigId, auto startTimestamp, auto endTimestamp )
      {
         ++numBoundCallbacks;
         
         // Get the name and type
         char type[512], name[512];
         (rig_getRigInfoDelegate(Utility::GetInstance()->GetFunctions()[ "rig_getRigInfo" ]))( "", rigId, "type", type, sizeof(type) );
         (rig_getRigInfoDelegate(Utility::GetInstance()->GetFunctions()[ "rig_getRigInfo" ]))( "", rigId, "name", name, sizeof(name) );

         CHECK( strlen(type) > 0 );
         CHECK( strlen(name) > 0 );

         (void)startTimestamp;
         (void)endTimestamp;
      };
      (rig_setBoundsCallbackDelegate(utility->GetFunctions()[ "rig_setBoundsCallback" ]))( boundsCallback );
      
      // Read should succeed
      returnValue = (rig_readDelegate(utility->GetFunctions()[ "rig_read" ]))( jsonFilename.c_str() );
      CHECK( returnValue == rig_NO_ERROR );
      CHECK( numBoundCallbacks > 0 );
      
      utility->CloseLib();
   }
   
   SECTION( "only_frame_callback" )
   {
      const std::string jsonFilename = g_url;
      Callbacks::numFrameCallbacks = 0;
      
      Utility * utility = Utility::GetInstance();
      
      REQUIRE_NOTHROW( utility->LoadLib() );
      
      rig_RETURN_CODE returnValue = rig_NO_ERROR;
      
      // Initialize the lib
      returnValue = (rig_initializeDelegate(utility->GetFunctions()[ "rig_initialize" ]))( nullptr );
      CHECK( returnValue == rig_NO_ERROR );
      
      (rig_setFrameCallbackDelegate(utility->GetFunctions()[ "rig_setFrameCallback" ]))( Callbacks::OnFrame );
      
      // Read should work
      returnValue = (rig_readDelegate(utility->GetFunctions()[ "rig_read" ]))( jsonFilename.c_str() );
      CHECK( returnValue == rig_NO_ERROR );
      CHECK( Callbacks::numFrameCallbacks > 0 );
      
      utility->CloseLib();
   }
}

TEST_CASE( "stress", "[stress]" )
{
   SECTION( "Synchronous" )
   {
      const std::string jsonFilename = g_url;
      
      Utility * utility = Utility::GetInstance();
      utility->LoadLib();
      
      rig_RETURN_CODE returnValue = rig_NO_ERROR;
      
      // Initialize the lib
      returnValue = (rig_initializeDelegate(utility->GetFunctions()[ "rig_initialize" ]))( nullptr );
      CHECK( returnValue == rig_NO_ERROR );
      
      // Error stuff
      rig_getLastErrorDelegate getLastError = (rig_getLastErrorDelegate(utility->GetFunctions()[ "rig_getLastError" ]));
      (rig_setErrorCallbackDelegate(utility->GetFunctions()[ "rig_setErrorCallback" ]))( Callbacks::OnError );
      
      // Data callbacks
      (rig_setBoundsCallbackDelegate(utility->GetFunctions()[ "rig_setBoundsCallback" ]))( [](auto rigId, auto startTime, auto endTime )
      { (void)rigId; (void)startTime; (void)endTime; } );
      (rig_setFrameCallbackDelegate(utility->GetFunctions()[ "rig_setFrameCallback" ]))( Callbacks::OnFrame );
      
      // Read synchronous
      (rig_readDelegate(utility->GetFunctions()[ "rig_read" ]))( jsonFilename.c_str() );
      
      // Get some info
      char fps[10];
      (rig_getInfoDelegate(Utility::GetInstance()->GetFunctions()[ "rig_getInfo" ]))( "", "fps", fps, sizeof(fps) );
      char version[ 20 ];
      (rig_getInfoDelegate(Utility::GetInstance()->GetFunctions()[ "rig_getInfo" ]))( "", "version", version, sizeof(version) );
      printf( "VERSION=%s, FPS=%s\n", version, fps );

      INFO( (std::string("Make sure ") + jsonFilename.c_str() + std::string(" exists")).c_str() )
      CHECK( (*getLastError)() == rig_NO_ERROR );

      (rig_uninitializeDelegate(utility->GetFunctions()[ "rig_uninitialize" ]))();
      
      utility->CloseLib();
      
      if ( !Callbacks::errorString.empty() )
         FAIL( Callbacks::errorString );
      Callbacks::errorString = "";
   }
   SECTION("asynchronous")
   {
      const std::string jsonFilename = g_url;
      
      Utility * utility = Utility::GetInstance();
      utility->LoadLib();

      rig_RETURN_CODE returnValue = rig_NO_ERROR;

      // Initialize the lib
      returnValue = (rig_initializeDelegate(utility->GetFunctions()["rig_initialize"]))(nullptr);
      CHECK(returnValue == rig_NO_ERROR);

      // Error stuff
      rig_getLastErrorDelegate getLastError = (rig_getLastErrorDelegate(utility->GetFunctions()["rig_getLastError"]));
      (rig_setErrorCallbackDelegate(utility->GetFunctions()["rig_setErrorCallback"]))(Callbacks::OnError);

      // Data callbacks
      (rig_setBoundsCallbackDelegate(utility->GetFunctions()["rig_setBoundsCallback"]))( [](auto rigId, auto startTime, auto endTime )
      { (void)rigId; (void)startTime; (void)endTime; } );
      (rig_setFrameCallbackDelegate(utility->GetFunctions()["rig_setFrameCallback"]))(Callbacks::OnFrame);

      // Read asynchronous
      {
         (rig_startReadDelegate(utility->GetFunctions()["rig_startRead"]))(jsonFilename.c_str());
         
         // Get some info
         char fps[10];
         (rig_getInfoDelegate(Utility::GetInstance()->GetFunctions()[ "rig_getInfo" ]))( "", "fps", fps, sizeof(fps) );
         INFO( fps )
         
         std::this_thread::sleep_for(std::chrono::milliseconds(1000));

         // Stop reading
         (rig_stopReadDelegate(utility->GetFunctions()["rig_stopRead"]))();
      }

      CHECK((*getLastError)() == rig_NO_ERROR);

      (rig_uninitializeDelegate(utility->GetFunctions()["rig_uninitialize"]))();

      utility->CloseLib();
      
      if ( !Callbacks::errorString.empty() )
         FAIL( Callbacks::errorString );
      Callbacks::errorString = "";
   }
   SECTION("no stop")
   {
      const std::string jsonFilename = g_url;

      Utility * utility = Utility::GetInstance();
      utility->LoadLib();

      rig_RETURN_CODE returnValue = rig_NO_ERROR;

      // Initialize the lib
      returnValue = (rig_initializeDelegate(utility->GetFunctions()["rig_initialize"]))(nullptr);
      CHECK(returnValue == rig_NO_ERROR);

      // Error stuff
      rig_getLastErrorDelegate getLastError = (rig_getLastErrorDelegate(utility->GetFunctions()["rig_getLastError"]));
      (rig_setErrorCallbackDelegate(utility->GetFunctions()["rig_setErrorCallback"]))(Callbacks::OnError);

      // Data callbacks
      (rig_setBoundsCallbackDelegate(utility->GetFunctions()["rig_setBoundsCallback"]))( [](auto rigId, auto startTime, auto endTime )
      { (void)rigId; (void)startTime; (void)endTime; } );
      (rig_setFrameCallbackDelegate(utility->GetFunctions()["rig_setFrameCallback"]))(Callbacks::OnFrame);

      // Read asynchronous but don't stop
      {
         (rig_startReadDelegate(utility->GetFunctions()["rig_startRead"]))(jsonFilename.c_str());
         std::this_thread::sleep_for(std::chrono::milliseconds(10));
      }
      
      CHECK((*getLastError)() == rig_NO_ERROR);

      // Should be able to handle this
      (rig_uninitializeDelegate(utility->GetFunctions()["rig_uninitialize"]))();
      CHECK( (*getLastError)() == rig_API_NOT_INITIALIZED );

      (rig_uninitializeDelegate(utility->GetFunctions()["rig_uninitialize"]))();

      utility->CloseLib();
   }
}
