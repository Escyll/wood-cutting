#pragma once

#include <string>
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "Renderer/Renderer.h"
#include "Renderer/Textures.h"

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

    void begin(unsigned int shaderId, unsigned int textureShaderId, RenderData renderData, Render::Camera* camera);
    void end();
    
    void panelBegin(const std::string& name, int x, int y, const LayoutStyle& layoutStyle, int padding = 4);
    void panelEnd();
    void beginLayout(const LayoutStyle& layoutStyle);
    void endLayout();
    bool button(const std::string& name, int width, int height);
    bool imageButton(const std::string& name, unsigned int image, const Frame& frame, int width, int height);

    void installCallbacks(GLFWwindow* window);
}
