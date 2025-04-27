#include "Imgui/Imgui.h"

#include <iostream>
#include <string>
#include <unordered_map>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace Imgui {

    struct Panel {
        std::string name;
        glm::vec4 color;
        int x = 0;
        int y = 0;
        int padding = 4;
        Layout layout;
    };

    struct Context {
        int mouseX;
        int mouseY;
        bool mouseDown;
        int previousMouseX;
        int previousMouseY;
        std::string activeElement;
        unsigned int shaderId;
        RenderData renderData;
        std::unordered_map<std::string, Panel> panels;
        std::string draggedPanel;
        std::string currentPanel;
    };
    Context context;

    void begin(unsigned int shaderId, RenderData renderData, int mouseX, int mouseY, bool mouseDown)
    {
        context.mouseX = mouseX;
        context.mouseY = mouseY;
        context.mouseDown = mouseDown;
        context.shaderId = shaderId;
        context.renderData = renderData;
        Render::flush();
        Render::printGLDebug("Rendering UI");
    }
    
    void end()
    {
        if (!context.mouseDown)
        {
            context.activeElement = "";
        }
        context.previousMouseX = context.mouseX;
        context.previousMouseY = context.mouseY;
        Render::flush();
    }

    void panelBegin(const std::string& name, const glm::vec4& color, int x, int y, const Layout& layout, int padding)
    {
        if (not context.panels.contains(name))
        {
            Panel panel = { name, color, x, y, padding, layout };
            context.panels[name] = panel;
        }
        context.currentPanel = name;
        context.panels[name].layout.elementCount = 0;
        context.panels[name].layout.width = 0;
        context.panels[name].layout.height = 0;
    }

    void panelEnd()
    {
        Panel& panel = context.panels[context.currentPanel];

        Render::Material material;
        material.name = panel.name;
        material.shader = context.shaderId;
        material.renderData = context.renderData;
        material.uniform4fs["color"] = panel.color;
        material.uniformMatrix4fvs["projection"] = glm::ortho(0.f, 1920.f, 1080.f, 0.f, -100.f, 100.f);
        material.uniformMatrix4fvs["view"] = glm::mat4(1.0f); // TODO This should be view now
        Render::setSubLayer(0);
        Render::setMaterial(material);

        Layout& layout = panel.layout;
        int x = panel.x;
        int y = panel.y;
        int width = 2*panel.padding + layout.width;
        int height = 2*panel.padding + layout.height;

        Render::queue({ {x, y}, {x + width, y}, {x + width, y + height}, {x + width, y + height}, {x, y + height}, {x, y} });
        context.currentPanel = "";
    }

    bool inBetween(int value, int min, int max)
    {
        return min <= value && value < max;
    }

    bool button(const std::string& name, int width, int height)
    {
        auto mouseX = context.mouseX;
        auto mouseY = context.mouseY;
        Panel& panel = context.panels[context.currentPanel];
        Layout& layout = panel.layout;
        int positionedX = layout.type == Layout::Column ? 0 : (layout.elementCount == 0 ? 0 : layout.width + layout.spacing);
        int positionedY = layout.type == Layout::Row ? 0 : (layout.elementCount == 0 ? 0 : (layout.height + layout.spacing));
        layout.height = layout.type == Layout::Column ? layout.height + (layout.elementCount == 0 ? height : (layout.spacing + height)) : std::max(layout.height, height);
        layout.width = layout.type == Layout::Row ? layout.width + (layout.elementCount == 0 ? width : layout.spacing + width) : std::max(layout.width, width);
        layout.elementCount++;
        int x = panel.x + panel.padding + positionedX;
        int y = panel.y + panel.padding + positionedY;

        bool underMouse = inBetween(mouseX, x, x + width) && inBetween(mouseY, y, y + height);
        if (underMouse && context.mouseDown && context.activeElement == "")
        {
            context.activeElement = name;
        }
        
        Render::Material material;
        material.name = name;
        material.shader = context.shaderId;
        material.renderData = context.renderData;
        glm::vec4 color { 0, 0, 1, 1 };
        if (underMouse)
        {
            color = { 0, 1, 0, 1 };
        }
        if (context.activeElement == name)
        {
            color = { 1, 0, 0, 1 };
        }
        material.uniform4fs["color"] = color;
        material.uniformMatrix4fvs["projection"] = glm::ortho(0.f, 1920.f, 1080.f, 0.f, -100.f, 100.f);
        material.uniformMatrix4fvs["view"] = glm::mat4(1.0f);

        Render::setSubLayer(1);
        Render::setMaterial(material);
        Render::queue({ {x, y}, {x + width, y}, {x + width, y + height}, {x + width, y + height}, {x, y + height}, {x, y} });

        return context.activeElement == name && underMouse && !context.mouseDown;
    }
}
