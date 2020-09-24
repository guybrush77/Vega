cmake_minimum_required(VERSION 3.14)

add_subdirectory(etna)
add_subdirectory(utils)

set(source_files main.cpp)

add_executable(vega)

target_sources(vega PRIVATE ${source_files})

target_link_libraries(
    vega PRIVATE
    etna
    spdlog
)

# IDE specific
set_target_properties(vega PROPERTIES FOLDER "src")
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${source_files})
