cmake_minimum_required(VERSION 3.14)

file(GLOB_RECURSE source_files *.hpp *.cpp)

add_executable(utest)

target_sources(utest PRIVATE ${source_files})

target_link_libraries(
    utest
    PRIVATE utils
    PRIVATE doctest
)

# IDE specific
set_target_properties(utest PROPERTIES FOLDER "tests")
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${source_files})
