cmake_minimum_required(VERSION 3.14)

# Test that resource auto generation (via make-resource tool) works as expected
set(test.resource.name test-resource)
set(test.resource.in   "${CMAKE_CURRENT_SOURCE_DIR}/resource.in.txt")
set(test.resource.out  "${CMAKE_CURRENT_BINARY_DIR}/resource.out.cpp")

add_custom_command(
    OUTPUT   ${test.resource.out}
    COMMAND  make-resource --resource ${test.resource.name} --input ${test.resource.in} --output ${test.resource.out}
    DEPENDS  make-resource ${test.resource.in}
    COMMENT "Making test resource"
)

add_executable(unit-tests)

# Gather source files
file(GLOB_RECURSE source_files *.hpp *.cpp)

target_sources(unit-tests PRIVATE ${source_files} ${test.resource.out})

target_link_libraries(
    unit-tests
    PRIVATE etna
    PRIVATE utils
    PRIVATE doctest
)

# IDE specific
get_directory_property(parent_path PARENT_DIRECTORY)
get_filename_component(parent_dir ${parent_path} NAME)

set_target_properties(unit-tests PROPERTIES FOLDER ${parent_dir})

source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${source_files})

source_group(autogen FILES ${test.resource.out})
