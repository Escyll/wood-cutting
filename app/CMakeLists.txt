project(app)

if(LINUX)
set(OpenGL_GL_PREFERENCE GLVND)
endif()
find_package(OpenGL REQUIRED)

add_executable(wood-cutting)

target_sources(wood-cutting PRIVATE
    main.cpp
    Catalog.h
    Geometry.h
    ECS/ECS.h
    ECS/Systems/Systems.h
    ECS/Systems/InputSystem.h
    Renderer/Renderer.h
    Renderer/Window.h
    Renderer/Shaders.h
    Renderer/Textures.h
    FontRendering/BMFont.h
)

target_link_libraries(wood-cutting PRIVATE glfw OpenGL::GL glad glm stb_image nlohmann_json::nlohmann_json)
target_include_directories(wood-cutting PRIVATE ${PROJECT_SOURCE_DIR})
