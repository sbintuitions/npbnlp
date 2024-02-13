cmake_minimum_required(VERSION 3.20.0)

set(CMAKE_VERBOSE_MAKEFILE true)
set(CMAKE_CXX_FLAGS_DEBUG_INIT "-Wall")
set(CMAKE_CXX_FLAGS_RELEASE_INIT "-O3 -Wall")
set(VERSION ${CMAKE_VERSION})
project(npbnlp CXX)

add_subdirectory(io)
add_subdirectory(rd)
add_subdirectory(lm)
add_subdirectory(ma)
add_subdirectory(tg)
add_subdirectory(pa)
