include(MakeFonts)

file(GLOB_RECURSE ttf_files RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} *.ttf)

get_filename_component(prefix ${CMAKE_CURRENT_SOURCE_DIR} NAME)

make_fonts("${ttf_files}" "${prefix}/" resource_files)

add_library(fonts OBJECT)

target_sources(fonts PRIVATE ${ttf_files} ${resource_files})

target_link_libraries(fonts PRIVATE utils)


# IDE specific

get_directory_property(parent_path PARENT_DIRECTORY)
get_filename_component(parent_dir ${parent_path} NAME)

set_target_properties(fonts PROPERTIES FOLDER ${parent_dir})

source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${ttf_files})
source_group(autogen FILES ${resource_files})
