project( kp2rig )
add_executable( kp2rig src/main.cpp )

# Copy our default kpDescriptor.json file to the binary directory, if it doesn't already exist there
if ( EXISTS '${BIN_DIR}/kpDescriptor.json')
else()
   file( COPY ${PROJECT_SOURCE_DIR}/kpDescriptor.json
      DESTINATION ${BIN_DIR} )
endif()

# Platform-specific stuff
if (WIN32)
    set( IPP_PATH "C:/Program Files (x86)/IntelSWTools/compilers_and_libraries/windows/ipp" CACHE PATH "Path to Intel IPP")
    set( IPP_LIB_SUBPATH "intel64_win"  CACHE STRING "Ipp library folder name under IPP_PATH")
elseif(APPLE)
    set( IPP_PATH "/opt/intel/ipp" CACHE PATH "Path to Intel IPP")
    set( IPP_LIB_SUBPATH "" )
else()
    set( IPP_PATH "/opt/intel/ipp" CACHE PATH "Base path to Intel IPP")
    set( IPP_LIB_SUBPATH "intel64" CACHE STRING "Ipp library folder name under IPP_PATH")
    find_package( Threads )
endif()

# Include IPP filtering?
if ( EXISTS ${IPP_PATH} )
   MESSAGE( "Found Intel Integrated Performance Primitives (${IPP_PATH})" )
   target_compile_definitions( kp2rig PRIVATE HAVE_IPP )
endif()

include_directories(
   ${PROJECT_SOURCE_DIR}/../common
   ${PROJECT_SOURCE_DIR}/../rig2c/include
   ${IPP_PATH}/include
   ${APR_UTIL_INCLUDE_DIRS}
   ${GLIB2_INCLUDE_DIRS} )

target_sources( kp2rig
   PRIVATE
      src/AnimatedRig.hpp
      src/AnimatedRig.cpp
      src/Animation.hpp
      src/Animation.cpp
      src/PoseFactory.hpp
      src/PoseFactory.cpp   
      src/KpMpii_16.hpp
      src/KpMpii_16.cpp
      src/KpMpii_20.hpp
      src/KpMpii_20.cpp
      src/KpMop_19.hpp
      src/KpMop_19.cpp
      src/KpSolidObject.hpp
      src/KpSolidObject.cpp
      src/KpHelper.hpp
      src/KpHelper.cpp
      src/KpToRigHelper.hpp
      src/KpToRigHelper.cpp
      src/RigToKpHelper.hpp
      src/RigToKpHelper.cpp
      src/RigPose.hpp
      src/RigPose.cpp
      src/Pose.hpp
      src/KpImporterFactory.hpp
      src/KpImporterFactory.cpp
      src/KpImporter.hpp
      src/KpCsvImporter.hpp
      src/KpCsvImporter.cpp
      src/KpCsvStreamImporter.hpp
      src/KpCsvStreamImporter.cpp
      src/KpJsonImporter.hpp
      src/KpJsonImporter.cpp
      src/KpType.hpp
      src/SmoothFactory.hpp
      src/SmoothFactory.cpp
      src/Smooth.hpp
      src/Smooth_lpfIpp.hpp
      src/Smooth_lpfIpp.cpp
      kpDescriptor.json
      ${PROJECT_SOURCE_DIR}/../common/config.h
      ${PROJECT_SOURCE_DIR}/../common/Rig.hpp
      ${PROJECT_SOURCE_DIR}/../common/Utility.hpp
      ${PROJECT_SOURCE_DIR}/../common/Utility.cpp
      ${PROJECT_SOURCE_DIR}/../common/Compression.hpp
      ${PROJECT_SOURCE_DIR}/../common/Compression.cpp )

if (WIN32)

   target_link_libraries( kp2rig Crypt32 )
   
elseif(APPLE)

   FIND_LIBRARY( FOUNDATION_FRAMEWORK Foundation )
   target_link_libraries( kp2rig ${FOUNDATION_FRAMEWORK} )
   
   target_sources( kp2rig
      PRIVATE
         ${PROJECT_SOURCE_DIR}/../common/Utility_Apple.mm
         ${PROJECT_SOURCE_DIR}/../common/BridgingHeader_Apple.h )
         
endif()

# Evaluates to nothing if specified utilities are not present or weren't asked for
target_link_libraries( kp2rig ${APR_UTIL_LIBRARIES} ${GLIB2_LIBRARIES} )

# I learned about generator expressions from here:
#   https://foonathan.net/2018/10/cmake-warnings/
target_compile_options( kp2rig
   PRIVATE
      $<$<OR:$<CXX_COMPILER_ID:Clang>,$<CXX_COMPILER_ID:AppleClang>,$<CXX_COMPILER_ID:GNU>>:
          -Werror -Wall -Wextra>
     $<$<CXX_COMPILER_ID:MSVC>:
          /W4> )
target_link_directories( kp2rig
   PRIVATE
      ${PROJECT_SOURCE_DIR}/../3rdparty/zfp/lib/${PLATFORM_STRING}
      ${IPP_PATH}/lib/${IPP_LIB_SUBPATH} )

target_link_libraries( kp2rig
   zfp
   ippcore
   ipps
   ${CMAKE_DL_LIBS}
   ${CMAKE_THREAD_LIBS_INIT} )
