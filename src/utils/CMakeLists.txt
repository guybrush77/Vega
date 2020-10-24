cmake_minimum_required(VERSION 3.14)

add_library(utils STATIC)

file(GLOB_RECURSE source_files *.hpp *.cpp)
target_sources(utils PRIVATE ${source_files})

target_compile_features(utils PUBLIC cxx_std_20)

target_include_directories(utils PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

# IDE specific
get_directory_property(parent_path PARENT_DIRECTORY)
get_filename_component(parent_dir ${parent_path} NAME)

set_target_properties(utils PROPERTIES FOLDER ${parent_dir})

source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${source_files})
