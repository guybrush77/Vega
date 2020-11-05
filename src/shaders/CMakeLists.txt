include(CompileShaders)

file(GLOB_RECURSE glsl_files RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} *.vert *.frag)

get_filename_component(prefix ${CMAKE_CURRENT_SOURCE_DIR} NAME)

compile_shaders("${glsl_files}" "${prefix}/" resource_files)

add_library(shaders OBJECT)

target_sources(shaders PRIVATE ${glsl_files} ${resource_files})

target_link_libraries(shaders PRIVATE utils)


# IDE specific

get_directory_property(parent_path PARENT_DIRECTORY)
get_filename_component(parent_dir ${parent_path} NAME)

set_target_properties(shaders PROPERTIES FOLDER ${parent_dir})

source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${glsl_files})
source_group(autogen FILES ${resource_files})
