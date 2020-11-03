cmake_minimum_required(VERSION 3.14)

add_library(vma INTERFACE)
target_include_directories(vma INTERFACE "${vma_SOURCE_DIR}/src")
