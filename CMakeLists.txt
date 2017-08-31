cmake_minimum_required(VERSION 3.5)
project(quake VERSION 1.0.0)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_subdirectory(original)

add_executable(quake dummy.cpp)
target_link_libraries(quake PUBLIC WinQuake)

