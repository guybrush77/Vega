cmake_minimum_required(VERSION 3.14)

add_subdirectory(utils)

add_executable(vega main.cpp)

target_link_libraries(
    vega PRIVATE
    glfw
    spdlog
)
