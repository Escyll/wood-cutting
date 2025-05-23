#pragma once

#include <string>
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "Renderer/Renderer.h"

namespace Imgui {

    struct LayoutStyle {
        enum Type {
            Column,
            Row,
            Grid
        };
        Type type;
        int spacing = 4;
    };

    void begin(unsigned int shaderId, RenderData renderData, Render::Camera* camera);
    void end();
    
    void panelBegin(const std::string& name, int x, int y, const LayoutStyle& layoutStyle, int padding = 4);
    void panelEnd();
    bool button(const std::string& name, int width, int height);

    void installCallbacks(GLFWwindow* window);
}
