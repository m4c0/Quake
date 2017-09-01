cmake_minimum_required(VERSION 3.5)
project(quake VERSION 1.0.0)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(quake PRIVATE includes)

add_subdirectory(original)

add_executable(quake
    source/common.cpp
    source/screen/screenshot.cpp
    )
target_include_directories(quake PRIVATE external/stb)
target_link_libraries(quake PUBLIC WinQuake)

