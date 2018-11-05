cmake_minimum_required(VERSION 3.11)
set(CMAKE_CXX_STANDARD 11)

project(test)

#### Setup Directories ####
#Main output directory
SET (CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/bin")
SET (CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/lib")
SET (CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/lib")

add_subdirectory("Lib/graphics/labs/framework")

#Glew is dumb and won't listen to our build directories, so manually set it.
set_target_properties(libglew_shared PROPERTIES
  RUNTIME_OUTPUT_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
  ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_ARCHIVE_OUTPUT_DIRECTORY}
  LIBRARY_OUTPUT_DIRECTORY ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}
)

add_executable(test main.cpp)
#include_directories(${CMAKE_SOURCE_DIR})

#add_subdirectory("lib/graphics/labs/framework")
target_include_directories(test PUBLIC "lib/graphics/labs/framework")#dependencies
target_link_libraries(test PRIVATE enu_graphics_framework )

#copy resources to build post build script
#add_custom_command(TARGET test POST_BUILD  
#COMMAND ${CMAKE_COMMAND} -E copy_directory  "${PROJECT_SOURCE_DIR}/res" $<TARGET_FILE_DIR:test>)


set_target_properties(enu_graphics_framework PROPERTIES FOLDER "DEPS")

#add_subdirectory("lib/something")
#target_include_directories(tset PUBLIC "lib/something")
