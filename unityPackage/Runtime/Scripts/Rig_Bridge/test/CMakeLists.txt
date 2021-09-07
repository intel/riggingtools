project( rig2csTest CSharp)

cmake_minimum_required(VERSION 3.10)

add_executable( rig2csTest
   rig2csTest.cs
   ${PROJECT_SOURCE_DIR}/../rig2c.cs
   ${PROJECT_SOURCE_DIR}/../librig2cInterop.cs )
