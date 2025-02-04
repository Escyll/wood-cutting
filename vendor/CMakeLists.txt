project(vendor)

include(ExternalProject)
include(FetchContent)

# GLFW
FetchContent_Declare(glfw GIT_REPOSITORY https://github.com/glfw/glfw.git GIT_TAG master)
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(glfw)

# GLM
FetchContent_Declare(glm GIT_REPOSITORY https://github.com/g-truc/glm.git GIT_TAG master)
FetchContent_MakeAvailable(glm)

# STB_IMAGE
set(STB_IMAGE_REPO_DIR "${CMAKE_CURRENT_BINARY_DIR}/stbImageRepo/")
ExternalProject_Add(stb_image-ext
    SOURCE_DIR "${STB_IMAGE_REPO_DIR}"
    GIT_REPOSITORY "https://github.com/nothings/stb.git"
    CONFIGURE_COMMAND ""
    BUILD_COMMAND ""
    INSTALL_COMMAND ""
    INSTALL_DIR ${CMAKE_SOURCE_DIR}
)
add_library(stb_image INTERFACE)
add_dependencies(stb_image stb_image-ext)
message("${STB_IMAGE_REPO_DIR}")
target_include_directories(stb_image INTERFACE "${STB_IMAGE_REPO_DIR}")

# glad
add_subdirectory(glad)

# imgui
add_subdirectory(imgui)
