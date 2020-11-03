cmake_minimum_required(VERSION 3.14)

find_package(Vulkan REQUIRED)

set(imgui_source_files
        "${imgui_SOURCE_DIR}/imconfig.h"
        "${imgui_SOURCE_DIR}/imgui.cpp"
        "${imgui_SOURCE_DIR}/imgui.h"
        "${imgui_SOURCE_DIR}/imgui_demo.cpp"
        "${imgui_SOURCE_DIR}/imgui_draw.cpp"
        "${imgui_SOURCE_DIR}/imgui_internal.h"
        "${imgui_SOURCE_DIR}/imgui_widgets.cpp"
        "${imgui_SOURCE_DIR}/imstb_rectpack.h"
        "${imgui_SOURCE_DIR}/imstb_textedit.h"
        "${imgui_SOURCE_DIR}/imstb_truetype.h"
        "${imgui_SOURCE_DIR}/examples/imgui_impl_glfw.cpp"
        "${imgui_SOURCE_DIR}/examples/imgui_impl_glfw.h"
        "${imgui_SOURCE_DIR}/examples/imgui_impl_vulkan.cpp"
        "${imgui_SOURCE_DIR}/examples/imgui_impl_vulkan.h")
        
add_library(imgui STATIC)

target_sources(imgui PRIVATE ${imgui_source_files})

target_include_directories(imgui PUBLIC "${imgui_SOURCE_DIR}")

target_link_libraries(
    imgui
    PRIVATE glfw
    PRIVATE Vulkan::Vulkan)
