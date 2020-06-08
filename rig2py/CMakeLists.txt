project( rig2py )
project( rig2pyBlender )
      
add_library( rig2py SHARED
   src/CInterface.h
   src/CInterface.c
   src/Joint.h
   src/Joint.c
   src/implementation.cpp
   ${PROJECT_SOURCE_DIR}/../common/Utility.hpp
   ${PROJECT_SOURCE_DIR}/../common/Utility.cpp
   ${PROJECT_SOURCE_DIR}/../common/Compression.hpp
   ${PROJECT_SOURCE_DIR}/../common/Compression.cpp
   ${PROJECT_SOURCE_DIR}/../rig2c/src/rig2c.cpp )

if (NOT WIN32)
   add_link_options( "-Wl,-undefined,dynamic_lookup" )
endif()

add_library( rig2pyBlender SHARED
   src/CInterface.h
   src/CInterface.c
   src/Joint.h
   src/Joint.c
   src/implementation.cpp
   ${PROJECT_SOURCE_DIR}/../common/Utility.hpp
   ${PROJECT_SOURCE_DIR}/../common/Utility.cpp
   ${PROJECT_SOURCE_DIR}/../common/Compression.hpp
   ${PROJECT_SOURCE_DIR}/../common/Compression.cpp
   ${PROJECT_SOURCE_DIR}/../rig2c/src/rig2c.cpp )
   
target_compile_definitions( rig2py PRIVATE -DMODULE_NAME=rig2py -DPy_LIMITED_API=0x03050000 )
target_compile_definitions( rig2pyBlender PRIVATE -DMODULE_NAME=rig2pyBlender )
   
include_directories( ${PROJECT_SOURCE_DIR}/../rig2c/include
   ${PROJECT_SOURCE_DIR}/../common
   ${APR_UTIL_INCLUDE_DIRS}
   ${GLIB2_INCLUDE_DIRS} )
target_include_directories( rig2pyBlender BEFORE PRIVATE ${PROJECT_SOURCE_DIR}/../3rdparty/python/include/${PLATFORM_STRING} )

target_link_libraries( rig2py ${Python3_LIBRARIES} )

# Evaluates to nothing if not present
target_link_libraries( rig2py ${APR_UTIL_LIBRARIES} ${GLIB2_LIBRARIES} )
target_link_libraries( rig2pyBlender ${APR_UTIL_LIBRARIES} ${GLIB2_LIBRARIES} )

if (WIN32)

   # Work-around for Python/Windows, because for some reason python force-includes python3.lib (relatively),
   # but cmake finds python3<minor-version>.lib (absolutely). Annoying!
   target_link_directories( rig2py PRIVATE ${Python3_LIBRARY_DIRS} )

   # More black-magic going on here: use a pre-defined def file to build a Windows lib file.
   # Yes, I could have included the lib but I'm trying to bundle as few binaries as possible,
   # so this text-based def file + the `lib` command should achieve what Windows needs.
   add_custom_command( TARGET rig2pyBlender
      PRE_LINK
      COMMAND genPyLib.bat
      BYPRODUCTS "python3.lib"
      WORKING_DIRECTORY "${PROJECT_SOURCE_DIR}/../rig2blender" )

   set_property(TARGET rig2py PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreadedDLL")
   set_property(TARGET rig2pyBlender PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreadedDLL")

   target_link_libraries( rig2py
      Crypt32
      ${PROJECT_SOURCE_DIR}/../3rdparty/zfp/lib/win64/zfp.lib )
   target_link_libraries( rig2pyBlender
      ${PROJECT_SOURCE_DIR}/../rig2blender/python3.lib
      Crypt32
      ${PROJECT_SOURCE_DIR}/../3rdparty/zfp/lib/win64/zfp.lib )
   set_property(TARGET rig2pyBlender PROPERTY CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /MD") # Yes, even for Debug. This is required for python interop.
   set_property(TARGET rig2py PROPERTY OUTPUT_NAME "rig2py.pyd")
   set_property(TARGET rig2pyBlender PROPERTY OUTPUT_NAME "rig2pyBlender.pyd")
   set_property(TARGET rig2py PROPERTY SUFFIX "")
   set_property(TARGET rig2pyBlender PROPERTY SUFFIX "")
   
elseif (APPLE)

   target_sources( rig2py
      PRIVATE
         ${PROJECT_SOURCE_DIR}/../common/Utility_Apple.mm
         ${PROJECT_SOURCE_DIR}/../common/BridgingHeader_Apple.h )
   target_sources( rig2pyBlender
      PRIVATE
         ${PROJECT_SOURCE_DIR}/../common/Utility_Apple.mm
         ${PROJECT_SOURCE_DIR}/../common/BridgingHeader_Apple.h )
         
   FIND_LIBRARY( FOUNDATION_FRAMEWORK Foundation )
         
   target_link_libraries( rig2py
      ${FOUNDATION_FRAMEWORK}
      ${PROJECT_SOURCE_DIR}/../3rdparty/zfp/lib/macosx/libzfp.a )
   target_link_libraries( rig2pyBlender
      ${FOUNDATION_FRAMEWORK}
      ${PROJECT_SOURCE_DIR}/../3rdparty/zfp/lib/macosx/libzfp.a )
   
   set_property(TARGET rig2py PROPERTY PREFIX "")
   set_property(TARGET rig2py PROPERTY OUTPUT_NAME "rig2py.so")
   set_property(TARGET rig2py PROPERTY SUFFIX "")
   set_property(TARGET rig2pyBlender PROPERTY PREFIX "")
   set_property(TARGET rig2pyBlender PROPERTY OUTPUT_NAME "rig2pyBlender.so")
   set_property(TARGET rig2pyBlender PROPERTY SUFFIX "")
   
else()
   
   target_link_libraries( rig2py
      ${PROJECT_SOURCE_DIR}/../3rdparty/zfp/lib/linux/libzfp.a )
   target_link_libraries( rig2pyBlender
      ${PROJECT_SOURCE_DIR}/../3rdparty/zfp/lib/linux/libzfp.a )
      
   target_compile_options( rig2py
      PRIVATE
         -Werror
         -Wall
         -Wextra )
   target_compile_options( rig2pyBlender
      PRIVATE
         -Werror
         -Wall
         -Wextra )
   set_property(TARGET rig2py PROPERTY PREFIX "")
   set_property(TARGET rig2pyBlender PROPERTY PREFIX "")
   
endif()
