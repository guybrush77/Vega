cmake_minimum_required(VERSION 3.14)

add_library(stb INTERFACE)

target_include_directories(stb INTERFACE "${stb_SOURCE_DIR}/..")
