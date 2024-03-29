cmake_minimum_required(VERSION 3.14)

include(FetchContent)

set(FETCHCONTENT_QUIET OFF)


#------------------------------------------------------------------------------------
# Fetch cxxopt
#------------------------------------------------------------------------------------

set(CXXOPTS_BUILD_EXAMPLES OFF)
set(CXXOPTS_BUILD_TESTS OFF)

FetchContent_Declare(
    cxxopt
    GIT_REPOSITORY  https://github.com/jarro2783/cxxopts
    GIT_TAG         v2.2.1
)

FetchContent_MakeAvailable(cxxopt)


#------------------------------------------------------------------------------------
# Fetch doctest
#------------------------------------------------------------------------------------

set(DOCTEST_WITH_TESTS OFF)
set(DOCTEST_NO_INSTALL ON)

FetchContent_Declare(
    doctest
    GIT_REPOSITORY  https://github.com/onqtam/doctest
    GIT_TAG         2.4.0
)

FetchContent_MakeAvailable(doctest)

set_target_properties(doctest_with_main PROPERTIES FOLDER "external")


#------------------------------------------------------------------------------------
# Fetch fmt
#------------------------------------------------------------------------------------

FetchContent_Declare(
    fmt
    GIT_REPOSITORY  https://github.com/fmtlib/fmt
    GIT_TAG         7.0.3
)

FetchContent_MakeAvailable(fmt)

set_target_properties(fmt PROPERTIES FOLDER "external")


#------------------------------------------------------------------------------------
# Fetch glfw
#------------------------------------------------------------------------------------

set(GLFW_BUILD_EXAMPLES OFF)
set(GLFW_BUILD_TESTS OFF)
set(GLFW_BUILD_DOCS OFF)

FetchContent_Declare(
    glfw
    GIT_REPOSITORY  https://github.com/glfw/glfw
    GIT_TAG         3.3.2
)

FetchContent_MakeAvailable(glfw)

set_target_properties(glfw PROPERTIES FOLDER "external/glfw")
set_target_properties(uninstall PROPERTIES FOLDER "external/glfw")


#------------------------------------------------------------------------------------
# Fetch glm
#------------------------------------------------------------------------------------

FetchContent_Declare(
    glm
    GIT_REPOSITORY  https://github.com/g-truc/glm
    GIT_TAG         0.9.9.8
)

FetchContent_MakeAvailable(glm)


#------------------------------------------------------------------------------------
# Fetch imgui
#------------------------------------------------------------------------------------

FetchContent_Declare(
    imgui
    GIT_REPOSITORY  https://github.com/ocornut/imgui
    GIT_TAG         v1.79
)

FetchContent_MakeAvailable(imgui)

include(imgui/imgui.cmake)

set_target_properties(imgui PROPERTIES FOLDER "external")


#------------------------------------------------------------------------------------
# Fetch json
#------------------------------------------------------------------------------------

set(JSON_BuildTests OFF)
set(JSON_Install OFF)

FetchContent_Declare(
    json
    GIT_REPOSITORY  https://github.com/nlohmann/json.git
    GIT_TAG         v3.9.1
)

FetchContent_MakeAvailable(json)

#------------------------------------------------------------------------------------
# Fetch spdlog
#------------------------------------------------------------------------------------

set(SPDLOG_FMT_EXTERNAL ON)

FetchContent_Declare(
    spdlog
    GIT_REPOSITORY  https://github.com/gabime/spdlog
    GIT_TAG         v1.8.0
)

FetchContent_MakeAvailable(spdlog)

set_target_properties(spdlog PROPERTIES FOLDER "external")


#------------------------------------------------------------------------------------
# Fetch stb
#------------------------------------------------------------------------------------

FetchContent_Declare(
    stb
    GIT_REPOSITORY  https://github.com/nothings/stb
    GIT_TAG         master
    SOURCE_DIR      ${CMAKE_BINARY_DIR}/_deps/stb-src/stb
)

FetchContent_MakeAvailable(stb)

include(stb/stb.cmake)


#------------------------------------------------------------------------------------
# Fetch vma
#------------------------------------------------------------------------------------

FetchContent_Declare(
    vma
    GIT_REPOSITORY  https://github.com/GPUOpen-LibrariesAndSDKs/VulkanMemoryAllocator
    GIT_TAG         v2.3.0
)

FetchContent_MakeAvailable(vma)

include(vma/vma.cmake)
