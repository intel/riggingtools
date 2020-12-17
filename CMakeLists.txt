# Native build from the project's root-folder:
#   cmake .
#   make
#
# Better yet, create a build folder and build from there:
#   mkdir build
#   cd build
#   cmake ..

cmake_minimum_required( VERSION 3.14 )

# Apple stuff to avoid a warning about rpaths
cmake_policy( SET CMP0042 NEW )

# Filename extensions must be explicit
if (${CMAKE_VERSION} VERSION_GREATER "3.20")
   cmake_policy( SET CMP0115 NEW )
endif()

# Target the 64-bit compiler on Windows. We assume 64-bit for everything
# and no effort has been made to support 32-bit.
if(WIN32)
   set( CMAKE_GENERATOR_PLATFORM x64 CACHE INTERNAL "Force 64-bit compilation" )
endif()

# The version number and C/C++ config header
project( kp2rig VERSION "0.5.0")
project( rig2py VERSION "0.5.0")
project( rig2c VERSION "0.5.0")
configure_file( "common/config.h.in" "common/config.h" )

# Main project
project( riggingTools )

# C++ settings
set( CMAKE_CXX_STANDARD 14 )
set( CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -DDEBUG")

include(CheckIncludeFile)
include(CheckIncludeFileCXX)

# Favor the system installation of catch2, but don't fail if it isn't installed
CHECK_INCLUDE_FILE_CXX( catch2/catch.hpp CATCH2_INSTALLED )
if(NOT CATCH2_INSTALLED)
   MESSAGE( "Using bundled catch2 from riggingtools/3rdParty" )
   include_directories( ${PROJECT_SOURCE_DIR}/3rdparty/catch2 )
endif()

# Favor the system installation of Eigen, but don't fail if it isn't installed
CHECK_INCLUDE_FILE_CXX( eigen3/Eigen/Geometry EIGEN3_INSTALLED )
if(NOT EIGEN3_INSTALLED)
   MESSAGE( "Using bundled eigen3 from riggingtools/3rdParty" )
   include_directories( ${PROJECT_SOURCE_DIR}/3rdparty/eigen3 )
endif()

# Add the eigen includes now that we know them
include_directories( ${Eigen3_INCLUDE_DIRS} )

# Ensure eigen is MPL-2.0
add_definitions( -DEIGEN_MPL2_ONLY )

# Favor the system installation of JSON for modern C++, but don't fail if it isn't installed
CHECK_INCLUDE_FILE_CXX( json.hpp JSON_FOR_MODERN_CXX_INSTALLED )
if(NOT JSON_FOR_MODERN_CXX_INSTALLED)
   MESSAGE( "Using bundled JSON for modern C++ from riggingtools/3rdParty" )
   include_directories( ${PROJECT_SOURCE_DIR}/3rdparty/nlohmann )
endif()

# Favor the system installation of CLI11, but don't fail if it isn't installed
CHECK_INCLUDE_FILE_CXX( CLI11.hpp CLI11_INSTALLED )
if(NOT CLI11_INSTALLED)
   MESSAGE( "Using bundled CLI11 from riggingtools/3rdParty" )
   include_directories( ${PROJECT_SOURCE_DIR}/3rdparty/CLI11 )
endif()

# We need a way to encode/decode base64. There are a lot of tools that can do this,
# but many of them have security concerns and/or portability issues.
# We're going to look for preferred tools in order of priority:
# - Foundation/NSData (macOS and iOS only)
# - Win32/Wincrypt (Windows only)
# - Apache Runtime Utilities
#
if (APPLE)
   SET( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D_USE_APPLE_NS_BASE64_" )
elseif(WIN32)
   SET( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D_USE_WIN32_BASE64_" )
else()
   find_path( APR_UTIL_INCLUDE_DIRS apu.h PATH_SUFFIXES apr-1 )
   find_library( APR_UTIL_LIBRARIES NAMES libaprutil-1 aprutil-1 )
   if ( APR_UTIL_LIBRARIES )
      SET( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D_USE_APR_BASE64_" )
      MESSAGE( "Using APR utilities for base64" )
   else()
      SET( APR_UTIL_INCLUDE_DIRS "" )
      SET( APR_UTIL_LIBRARIES "" )
      find_path( GLIB2_INCLUDE_DIRS glib.h PATH_SUFFIXES glib-2.0 )
      find_library( GLIB2_LIBRARIES NAMES glib-2.0 )
      if ( GLIB2_LIBRARIES )
         execute_process( COMMAND pkg-config --cflags-only-I glib-2.0 OUTPUT_VARIABLE GLIB_CFLAGS OUTPUT_STRIP_TRAILING_WHITESPACE )
	 SET( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${GLIB_CFLAGS} -D_USE_GLIB_BASE64_" )
	 MESSAGE( "Using GLIB2 for base64" )
      else()
         MESSAGE( FATAL_ERROR "Could not find base64 implementation (install glib2 or APR-util)" )
      endif()
   endif()
endif()

# Favor the system installation of ZFP, but don't fail if it isn't installed
# TODO: ZFP isn't standard on most systems so this check is near worthless -
#       the bundled one will be used most of the time.
#       I have no recommendation for an alternative so it is what it is.
CHECK_INCLUDE_FILE( zfp.h ZFP_HEADER_INSTALLED )
if(NOT ZFP_HEADER_INSTALLED)
   MESSAGE( "Using bundled ZFP from riggingtools/3rdParty" )
   include_directories( ${PROJECT_SOURCE_DIR}/3rdparty/zfp/include )
endif()

# Put all cmake files in their own folders
#set( CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake" ${CMAKE_MODULE_PATH} )

if (APPLE)
   if (CMAKE_SYSTEM_NAME STREQUAL "iOS")
      set( PLATFORM_STRING "iphoneos" )
   else()
      set( PLATFORM_STRING "macosx" )
   endif()
   
   # Allow libs not part of this build to be used for linking
   cmake_policy( SET CMP0079 NEW )
elseif(WIN32)
   set( PLATFORM_STRING "win64" )
elseif(ANDROID)
   set( PLATFORM_STRING "armv7a" )
else()
   set( PLATFORM_STRING "linux" )
endif()

# Force all output binaries to a single directory.
# The generator expression is used to avoid IDEs from adding a suffix to the output path,
# such as "Debug" or "Release".
# See here for more info: https://cmake.org/cmake/help/latest/prop_tgt/LIBRARY_OUTPUT_DIRECTORY.html
set( BIN_DIR  ${PROJECT_SOURCE_DIR}/bin/${PLATFORM_STRING} )
set( CMAKE_RUNTIME_OUTPUT_DIRECTORY         ${BIN_DIR}$<0:> )
set( CMAKE_LIBRARY_OUTPUT_DIRECTORY         ${BIN_DIR}$<0:> )
set( CMAKE_ARCHIVE_OUTPUT_DIRECTORY         ${BIN_DIR}$<0:> )

# Every platform supports rig2c
add_subdirectory( rig2c )
install( TARGETS rig2c DESTINATION lib )

# Only some platforms include server-side and python
if ( (${PLATFORM_STRING} STREQUAL "linux") OR (${PLATFORM_STRING} STREQUAL "win64") OR (${PLATFORM_STRING} STREQUAL "macosx") )

   add_subdirectory( kp2rig )
   install( TARGETS kp2rig DESTINATION bin )

   # Include python be default, or if '-DWITH_PYTHON=YES' is passed in
   if ( (NOT DEFINED WITH_PYTHON) OR
      (WITH_PYTHON STREQUAL "YES") OR
      (WITH_PYTHON STREQUAL "yes") OR
      (WITH_PYTHON STREQUAL "Y") OR
      (WITH_PYTHON STREQUAL "y") OR
      (WITH_PYTHON STREQUAL "1") )
      
      # Find the system-installed python
      find_package( Python3 COMPONENTS Development )
      if(NOT Python3_INCLUDE_DIRS)
         MESSAGE( SEND_ERROR "Please install python3 development libraries, or disable python3 with '-DWITH_PYTHON=NO'" )
      endif()
      
      # Add the python3 includes now that we know them
      include_directories( ${Python3_INCLUDE_DIRS} )
      
      add_subdirectory( rig2py )
      
   endif()
   
endif()

# Add the C# unit test for Windows
if(WIN32)
   add_subdirectory( rig2cs )
endif()

# Add the swift framework for iOS
if ( APPLE )
   if ( ${CMAKE_GENERATOR} STREQUAL "Xcode" )
      add_subdirectory( rig2swift )
   else()
      MESSAGE( "\n INFO: rig2swift can only be built with the '-G Xcode' generator\n" )
   endif()
endif()
