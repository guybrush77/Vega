cmake_minimum_required(VERSION 3.14)


if (MSVC)
    set (WARNING_FLAGS "/W3 /permissive-")
else()
    set (WARNING_FLAGS "-Werror -Wall -Wextra -Wconversion -Wsign-conversion -pedantic-errors")
endif()

add_subdirectory(etna)
add_subdirectory(shaders)
add_subdirectory(utils)
add_subdirectory(vega)
