cmake_minimum_required(VERSION 3.11)
project(test)

set(CMAKE_CXX_STANDARD 11)

add_executable(test main.cpp)
#include_directories(${CMAKE_SOURCE_DIR})

add_subdirectory("lib/graphics/labs/framework")
target_include_directories(test PUBLIC "lib/graphics/labs/framework")

#add_subdirectory("lib/something")
#target_include_directories(tset PUBLIC "lib/something")
