cmake_minimum_required(VERSION 3.14)

add_executable(make-resource)

set(source_files make_resource.cpp)

target_sources(make-resource PRIVATE ${source_files})

target_link_libraries(make-resource PRIVATE cxxopts fmt)


# IDE specific

get_directory_property(parent_path PARENT_DIRECTORY)
get_filename_component(parent_dir ${parent_path} NAME)

set_target_properties(make-resource PROPERTIES FOLDER ${parent_dir})

source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${source_files})
