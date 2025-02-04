project(app)

if(LINUX)
set(OpenGL_GL_PREFERENCE GLVND)
endif()
find_package(OpenGL REQUIRED)

add_executable(wood-cutting)

target_sources(wood-cutting PRIVATE
    main.cpp
    Geometry.h
    ECS/ECS.h
    ECS/Systems/Systems.h
    Renderer/Renderer.h
    Renderer/Renderer.cpp
    Renderer/Shaders.h
)

target_link_libraries(wood-cutting PRIVATE glfw OpenGL::GL glad glm)
target_include_directories(wood-cutting PRIVATE ${PROJECT_SOURCE_DIR})
