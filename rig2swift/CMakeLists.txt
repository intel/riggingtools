# Required due to a bug in earlier versions
cmake_minimum_required( VERSION 3.16 )

# This bypasses checking for a swift compiler, which fails on recent
# versions of Xcode (as of Jun 2020)
set( CMAKE_Swift_COMPILER_FORCED 1 )

project( rig2swift Swift )

include_directories(
   ${PROJECT_SOURCE_DIR}/../rig2c/include )

set( BRIDGING_HEADER "${PROJECT_SOURCE_DIR}/rig2swift-Bridging-Header.h" )

add_library( rig2swift STATIC
   rig2swift.swift
   ${BRIDGING_HEADER} )

target_link_libraries( rig2swift PRIVATE
   rig2c
   c++ )

set_target_properties( rig2swift PROPERTIES
   FRAMEWORK TRUE
   XCODE_ATTRIBUTE_SWIFT_OBJC_BRIDGING_HEADER ${BRIDGING_HEADER}
   PRIVATE_HEADER ${BRIDGING_HEADER}
   XCODE_ATTRIBUTE_TARGETED_DEVICE_FAMILY "1,2" )

# Add the unit test
if ( ${PLATFORM_STRING} STREQUAL "iphoneos" )
   add_subdirectory( testIos )
endif()
