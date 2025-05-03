project(app)

if(LINUX)
set(OpenGL_GL_PREFERENCE GLVND)
endif()
find_package(OpenGL REQUIRED)

add_executable(wood-cutting)

target_sources(wood-cutting
    PRIVATE
        main.cpp
        Catalog.h
        Catalog.cpp
        Geometry.h
        ECS/ECS.h
        ECS/Systems/InputSystem.h
        ECS/Systems/InputSystem.cpp
        ECS/Systems/Systems.h
        ECS/Systems/Systems.cpp
        Renderer/Camera.h
        Renderer/Camera.cpp
        Renderer/Renderer.h
        Renderer/Renderer.cpp
        Renderer/Window.h
        Renderer/Shaders.h
        Renderer/Textures.h
        Renderer/Textures.cpp
        FontRendering/BMFont.h
        FontRendering/BMFont.cpp
        Platform.h
        Platform.cpp
        Imgui/Imgui.h
        Imgui/Imgui.cpp
)

target_link_libraries(wood-cutting PRIVATE glfw OpenGL::GL glad glm stb_image nlohmann_json::nlohmann_json)
target_include_directories(wood-cutting PRIVATE ${PROJECT_SOURCE_DIR})
