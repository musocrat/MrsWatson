cmake_minimum_required(VERSION 2.6)
project(mrswatsoncore)

file(GLOB mrswatsoncore_SOURCES *.c */*.c */*.cpp)

# On unix, we can build both the 32/64 bit versions at once. However with
# Visual Studio we need to generate two separate out-of-source build dirs,
# one for each architecture.

if(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
  add_library(mrswatsoncore ${mrswatsoncore_SOURCES})
  set_target_properties(mrswatsoncore PROPERTIES COMPILE_FLAGS "-m32")
  set_target_properties(mrswatsoncore PROPERTIES LINK_FLAGS "-m32")
elseif(APPLE)
  file(GLOB mrswatsoncore_OBJCSOURCES *.m *.mm */*.m */*.mm)
  add_library(mrswatsoncore ${mrswatsoncore_SOURCES} ${mrswatsoncore_OBJCSOURCES})
  set_source_files_properties(${mrswatsoncore_OBJCSOURCES} PROPERTIES COMPILE_FLAGS "-x objective-c++")
  set_target_properties(mrswatsoncore PROPERTIES COMPILE_FLAGS "-arch i386")
  set_target_properties(mrswatsoncore PROPERTIES LINK_FLAGS "-arch i386")
elseif(MSVC)
  if(${platform_bits} EQUAL 32)
    add_library(mrswatsoncore ${mrswatsoncore_SOURCES})
    set_target_properties(mrswatsoncore PROPERTIES COMPILE_FLAGS "/D WIN32=1")
  endif()
endif()

if(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
  add_library(mrswatsoncore64 ${mrswatsoncore_SOURCES})
  set_target_properties(mrswatsoncore64 PROPERTIES COMPILE_FLAGS "-m64 -DWITH_64BIT=1")
  set_target_properties(mrswatsoncore64 PROPERTIES LINK_FLAGS "-m64 -DWITH_64BIT=1")
elseif(APPLE)
  file(GLOB mrswatsoncore_OBJCSOURCES *.m *.mm */*.m */*.mm)
  add_library(mrswatsoncore64 ${mrswatsoncore_SOURCES} ${mrswatsoncore_OBJCSOURCES})
  set_source_files_properties(${mrswatsoncore_OBJCSOURCES} PROPERTIES COMPILE_FLAGS "-x objective-c++")
  set_target_properties(mrswatsoncore64 PROPERTIES COMPILE_FLAGS "-arch x86_64 -DWITH_64BIT=1")
  set_target_properties(mrswatsoncore64 PROPERTIES LINK_FLAGS "-arch x86_64 -DWITH_64BIT=1")
elseif(MSVC)
  if(${platform_bits} EQUAL 64)
    add_library(mrswatsoncore64 ${mrswatsoncore_SOURCES})
    set_target_properties(mrswatsoncore64 PROPERTIES COMPILE_FLAGS "/MACHINE:X64 /D WIN64=1 /D WITH64_BIT=1")
  endif()
endif()

