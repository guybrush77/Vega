cmake_minimum_required(VERSION 3.14)

add_executable(vega)

set(source_files vega.cpp)
target_sources(vega PRIVATE ${source_files})

target_compile_features(vega PUBLIC cxx_std_20)

target_link_libraries(
    vega
    PRIVATE etna
    PRIVATE glfw
    PRIVATE glm
    PRIVATE shaders
    PRIVATE spdlog
    PRIVATE utils
)

# IDE specific
get_directory_property(parent_path PARENT_DIRECTORY)
get_filename_component(parent_dir ${parent_path} NAME)

set_target_properties(vega PROPERTIES FOLDER ${parent_dir})

source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${source_files})
