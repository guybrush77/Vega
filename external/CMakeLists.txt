cmake_minimum_required(VERSION 3.14)

include(FetchContent)

set(FETCHCONTENT_QUIET OFF)

#------------------------------------------------------------------------------------
# Fetch glfw
#------------------------------------------------------------------------------------

set(GLFW_BUILD_DOCS OFF)
set(GLFW_BUILD_TESTS OFF)
set(GLFW_BUILD_EXAMPLES OFF)

FetchContent_Declare(
    glfw
    GIT_REPOSITORY  https://github.com/glfw/glfw
    GIT_TAG         3.3.2
)

FetchContent_MakeAvailable(glfw)


#------------------------------------------------------------------------------------
# Fetch spdlog
#------------------------------------------------------------------------------------

FetchContent_Declare(
    spdlog
    GIT_REPOSITORY  https://github.com/gabime/spdlog
    GIT_TAG         v1.8.0
)

FetchContent_MakeAvailable(spdlog)

