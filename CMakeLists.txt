cmake_minimum_required(VERSION 3.14)
project(media)

set(CMAKE_CXX_STANDARD 17)

# SDL2 Manual Setup
set(SDL2_INCLUDE_DIR "${CMAKE_SOURCE_DIR}/third_party/SDL2/include")
set(SDL2_LIBRARY_DIR "${CMAKE_SOURCE_DIR}/third_party/SDL2/lib/x64")

include_directories(${SDL2_INCLUDE_DIR})
link_directories(${SDL2_LIBRARY_DIR})

# ImGui source files
file(GLOB IMGUI_SRC
    ImGui/*.cpp
    ImGui/backends/imgui_impl_sdl2.cpp
    ImGui/backends/imgui_impl_opengl3.cpp
)

# Executable
include_directories(${CMAKE_SOURCE_DIR}/third_party/gl3w)
add_executable(media
    media.cpp
    third_party/gl3w/gl3w.c
    ${IMGUI_SRC}
 "resources.h")

# VLC paths
include_directories("${CMAKE_SOURCE_DIR}/include")
link_directories("${CMAKE_SOURCE_DIR}/lib")

# Include ImGui headers
target_include_directories(media PRIVATE
    ImGui
    ImGui/backends
)

# Link libraries
target_link_libraries(media
    "${CMAKE_SOURCE_DIR}/lib/libvlc.lib"
    "${CMAKE_SOURCE_DIR}/lib/libvlccore.lib"
    SDL2
    SDL2main
)


# Define for SDL2 main on Windows
if(WIN32)
    target_compile_definitions(media PRIVATE SDL_MAIN_HANDLED)
endif()

# Add resource file for Windows
if(WIN32)
    target_sources(media PRIVATE resources.rc)
endif()

message(STATUS "Looking for libvlc.lib at: ${CMAKE_SOURCE_DIR}/lib/libvlc.lib")

