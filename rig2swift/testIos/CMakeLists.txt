
# Required due to a bug in earlier versions
cmake_minimum_required( VERSION 3.16 )

# This bypasses checking for a swift compiler, which fails on recent
# versions of Xcode (as of Jun 2020)
set( CMAKE_Swift_COMPILER_FORCED 1 )

project( testIos Swift )

add_executable( testIos
   MACOSX_BUNDLE
   AppDelegate.swift
   ViewController.swift
   SceneDelegate.swift )
   
target_link_libraries( testIos PRIVATE
   rig2swift )

set_target_properties( testIos PROPERTIES MACOSX_BUNDLE_INFO_PLIST "${PROJECT_SOURCE_DIR}/Info.plist" )

# Disable code signing, but a development team is still required
set( CMAKE_XCODE_ATTRIBUTE_CODE_SIGNING_REQUIRED "NO" )
set( CMAKE_XCODE_ATTRIBUTE_CODE_SIGN_IDENTITY "" )

# Target both iPhone and iPad
set_target_properties( testIos PROPERTIES XCODE_ATTRIBUTE_TARGETED_DEVICE_FAMILY "1,2" )
