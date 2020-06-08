project( rig2c )

include_directories(
   ${PROJECT_SOURCE_DIR}/include
   ${PROJECT_SOURCE_DIR}/../common
   ${APR_UTIL_INCLUDE_DIRS}
   ${GLIB2_INCLUDE_DIRS} )

add_definitions( -DRIG_API_EXPORTS )

if (APPLE)
   
   if ( ${PLATFORM_STRING} STREQUAL "iphoneos" )
      add_library( rig2c STATIC
         src/rig2c.cpp )

      target_link_libraries( rig2c
         ${PROJECT_SOURCE_DIR}/../3rdparty/zfp/lib/iphoneos/libzfp.a )

   else()
      FIND_LIBRARY( FOUNDATION_FRAMEWORK Foundation )
      
      add_library( rig2c SHARED
         src/rig2c.cpp )

      target_link_libraries( rig2c
         ${PROJECT_SOURCE_DIR}/../3rdparty/zfp/lib/macosx/libzfp.a
         ${FOUNDATION_FRAMEWORK} )

      set_property(TARGET rig2c PROPERTY OUTPUT_NAME "rig2c.so")
      set_property(TARGET rig2c PROPERTY SUFFIX "")
      
      # Let's make a bundle also so we can easily plug-in to Unity (though rumor has it
      # you can simply rename the .so to .bundle and Unity is happy)
      project( rig2c_bundle )

      add_library( rig2c_bundle MODULE
         src/rig2c.cpp
         include/rig2c.h
         include/rig2cConfig.h
         include/rig2cDelegates.h
         include/rig2cTypes.h
         ${PROJECT_SOURCE_DIR}/../common/config.h
         ${PROJECT_SOURCE_DIR}/../common/Rig.hpp
         ${PROJECT_SOURCE_DIR}/../common/Utility.hpp
         ${PROJECT_SOURCE_DIR}/../common/Utility.cpp
         ${PROJECT_SOURCE_DIR}/../common/Compression.hpp
         ${PROJECT_SOURCE_DIR}/../common/Compression.cpp
         ${PROJECT_SOURCE_DIR}/../common/Utility_Apple.mm
         ${PROJECT_SOURCE_DIR}/../common/BridgingHeader_Apple.h )

      target_link_libraries( rig2c_bundle
         ${PROJECT_SOURCE_DIR}/../3rdparty/zfp/lib/macosx/libzfp.a
         ${APR_UTIL_LIBRARIES}
         ${FOUNDATION_FRAMEWORK} )
         
      set_target_properties(rig2c_bundle PROPERTIES BUNDLE TRUE )
      set_target_properties(rig2c_bundle PROPERTIES MACOSX_BUNDLE_BUNDLE_NAME "rig2c" )
      set_property(TARGET rig2c_bundle PROPERTY OUTPUT_NAME "rig2c")
      
   endif()
   
   set_target_properties( rig2c PROPERTIES MACOSX_BUNDLE_GUI_IDENTIFIER "com.intel.rig2c" )
   set_target_properties( rig2c PROPERTIES MACOSX_FRAMEWORK_IDENTIFIER "com.intel.rig2c" )
   set_target_properties( rig2c PROPERTIES MACOSX_BUNDLE_INFO_STRING "Client API for reading rig files" )
   
   target_sources( rig2c
      PRIVATE
         ${PROJECT_SOURCE_DIR}/../common/Utility_Apple.mm
         ${PROJECT_SOURCE_DIR}/../common/BridgingHeader_Apple.h )
   
else()
   add_library( rig2c SHARED
      src/rig2c.cpp )

   if (WIN32)
      target_link_libraries( rig2c
         Crypt32
         ${PROJECT_SOURCE_DIR}/../3rdparty/zfp/lib/${PLATFORM_STRING}/zfp.lib )
   else()
      target_link_libraries( rig2c
         ${PROJECT_SOURCE_DIR}/../3rdparty/zfp/lib/${PLATFORM_STRING}/libzfp.a )
      target_compile_options( rig2c
          PRIVATE
             -Werror
             -Wall
             -Wextra )
   endif()
endif()

target_sources( rig2c
   PRIVATE
      include/rig2c.h
      include/rig2cConfig.h
      include/rig2cDelegates.h
      include/rig2cTypes.h
      ${PROJECT_SOURCE_DIR}/../common/config.h
      ${PROJECT_SOURCE_DIR}/../common/Rig.hpp
      ${PROJECT_SOURCE_DIR}/../common/Utility.hpp
      ${PROJECT_SOURCE_DIR}/../common/Utility.cpp
      ${PROJECT_SOURCE_DIR}/../common/Compression.hpp
      ${PROJECT_SOURCE_DIR}/../common/Compression.cpp )

# Evaluates to nothing if APR utilities are not present
target_link_libraries( rig2c ${APR_UTIL_LIBRARIES} ${GLIB2_LIBRARIES} )

# Add the test
if ( (${PLATFORM_STRING} STREQUAL "linux") OR (${PLATFORM_STRING} STREQUAL "win64") OR (${PLATFORM_STRING} STREQUAL "macosx") )
   add_subdirectory( test )
endif()
