cmake_minimum_required(VERSION 3.14)

find_package(Vulkan REQUIRED)

add_library(etna STATIC)

file(GLOB_RECURSE source_files *.hpp *.cpp)

target_sources(etna PRIVATE ${source_files})

target_include_directories(etna PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(
    etna
    PRIVATE utils
    PUBLIC  Vulkan::Vulkan)

# IDE specific
set_target_properties(etna PROPERTIES FOLDER "src")
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${source_files})
