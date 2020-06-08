project( rig2cTest )

include_directories(
   ${PROJECT_SOURCE_DIR}/../../rig2c/include
   ${PROJECT_SOURCE_DIR}/../../common )

add_executable( rig2cTest
   ${PROJECT_SOURCE_DIR}/../../common/Utility.cpp
   src/main.cpp
   src/StressTest.cpp
   src/Callbacks )
 
# Needed for iOS, meaningless for others
set_target_properties( rig2cTest PROPERTIES MACOSX_BUNDLE_GUI_IDENTIFIER "com.intel.rig2cTest" )
set_target_properties( rig2cTest PROPERTIES MACOSX_FRAMEWORK_IDENTIFIER "com.intel.rig2cTest" )
set_target_properties( rig2cTest PROPERTIES MACOSX_BUNDLE_INFO_STRING "Unit test for rig2c" )

if(UNIX AND NOT APPLE)
   target_link_libraries( rig2cTest
      dl pthread )
   target_compile_options( rig2cTest
      PRIVATE
         -Werror
         -Wall
         -Wextra )
endif()
