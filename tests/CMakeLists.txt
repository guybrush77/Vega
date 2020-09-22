cmake_minimum_required(VERSION 3.14)

add_executable(vega_ut test_main.cpp)

target_link_libraries(
    vega_ut PRIVATE
    doctest
)
