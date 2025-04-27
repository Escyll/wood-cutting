#pragma once

#include <string>

#include "Renderer/Renderer.h"

namespace Imgui {

    struct Layout {
        enum Type {
            Column,
            Row,
            Grid
        };
        Type type;
        int spacing = 4;

        int rowCount = 0;
        int columnCount = 0;
        int width = 0;
        int height = 0;
        int elementCount = 0;
    };


    void begin(unsigned int shaderId, RenderData renderData, int mouseX, int mouseY, bool mouseDown);
    void end();
    
    void panelBegin(const std::string& name, const glm::vec4& color, int x, int y, const Layout& layout, int padding = 4);
    void panelEnd();
    bool button(const std::string& name, int width, int height);
}
