cmake_minimum_required(VERSION 3.14)

find_package(Vulkan REQUIRED)

add_library(etna STATIC)

file(GLOB_RECURSE sources *.hpp *.cpp)

target_sources(etna PRIVATE ${sources})

target_include_directories(etna PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(
    etna
    PRIVATE vega_utils
    PUBLIC  Vulkan::Vulkan)
