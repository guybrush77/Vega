cmake_minimum_required(VERSION 3.10)

project(Vega LANGUAGES CXX)

add_subdirectory(src)

set_property(DIRECTORY ${CMAKE_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT vega)
