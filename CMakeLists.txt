cmake_minimum_required(VERSION 3.8)
project(quake VERSION 1.0.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(quake PRIVATE includes)

add_subdirectory(original)

add_executable(quake
    source/cmd.cpp
    source/common.cpp
    source/screen/screenshot.cpp
    source/wad.cpp
    )
target_include_directories(quake PRIVATE external/stb)
target_link_libraries(quake PUBLIC WinQuake)

