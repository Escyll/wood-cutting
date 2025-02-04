if(LINUX)
set(OpenGL_GL_PREFERENCE GLVND)
endif()
find_package(OpenGL REQUIRED)

set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)
add_library(imgui STATIC)
target_sources(imgui PRIVATE
"imgui/imgui_draw.cpp"
"imgui/imgui_tables.cpp"
"imgui/imgui_widgets.cpp"
"imgui/imgui.cpp"
"imgui/backends/imgui_impl_glfw.cpp"
"imgui/backends/imgui_impl_opengl3.cpp")
target_include_directories(imgui PUBLIC imgui)
target_link_libraries(imgui PRIVATE glfw OpenGL::GL glad)
