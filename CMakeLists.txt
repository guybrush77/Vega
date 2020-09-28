cmake_minimum_required(VERSION 3.14)

project(Vega LANGUAGES CXX)

set(CMAKE_POLICY_DEFAULT_CMP0077 NEW)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/tools/cmake")

add_subdirectory(external)
add_subdirectory(src)
add_subdirectory(tests)
add_subdirectory(tools)

set_property(DIRECTORY ${CMAKE_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT vega)
