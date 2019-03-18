cmake_minimum_required(VERSION 3.11)
set(CMAKE_CXX_STANDARD 11)

project(Visualiser)

#### Setup Directories ####
#Main output directory
SET (CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/bin")
SET (CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/lib")
SET (CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/lib")

add_subdirectory("Lib/graphics")
add_subdirectory("Lib/portaudio")
add_subdirectory("Lib/Gist")


#Glew is dumb and won't listen to our build directories, so manually set it.
set_target_properties(libglew_shared PROPERTIES
  RUNTIME_OUTPUT_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
  ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_ARCHIVE_OUTPUT_DIRECTORY}
  LIBRARY_OUTPUT_DIRECTORY ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}
)

add_executable(Visualiser main.cpp audio_handler.h audio_handler.cpp)
target_link_libraries(Visualiser PRIVATE enu_graphics_framework portaudio_static Gist)


add_custom_target(copy_res ALL COMMAND ${CMAKE_COMMAND} -E copy_directory "${PROJECT_SOURCE_DIR}/res" "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/$<CONFIG>/res")


set_target_properties(enu_graphics_framework PROPERTIES FOLDER "DEPS")
