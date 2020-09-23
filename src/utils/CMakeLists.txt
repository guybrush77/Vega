cmake_minimum_required(VERSION 3.14)

add_library(vega_utils STATIC)

file(GLOB_RECURSE sources *.hpp *.cpp)

target_sources(vega_utils PRIVATE ${sources})

target_include_directories(vega_utils PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
