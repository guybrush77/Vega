cmake_minimum_required(VERSION 3.14)

add_library(utils STATIC)

file(GLOB_RECURSE source_files *.hpp *.cpp)

target_sources(utils PRIVATE ${source_files})

target_include_directories(utils PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

# IDE specific
set_target_properties(utils PROPERTIES FOLDER "src")
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${source_files})
