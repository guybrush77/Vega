cmake_minimum_required(VERSION 3.14)

find_package(Vulkan REQUIRED)

add_library(etna STATIC)

file(GLOB_RECURSE source_files *.hpp *.cpp)
target_sources(etna PRIVATE ${source_files})

target_compile_features(etna PUBLIC cxx_std_20)

target_include_directories(etna PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include")

target_link_libraries(
    etna
    PRIVATE spdlog
    PRIVATE vma
    PUBLIC  Vulkan::Vulkan)

# IDE specific
get_directory_property(parent_path PARENT_DIRECTORY)
get_filename_component(parent_dir ${parent_path} NAME)

set_target_properties(etna PROPERTIES FOLDER ${parent_dir})

source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${source_files})
