#include "Imgui/Imgui.h"

#include <iostream>
#include <string>
#include <unordered_map>
#include <queue>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace Imgui {
    
    struct Pos {
        int x;
        int y;
        Pos& operator+=(const Pos& rhs)
        {
            x += rhs.x;
            y += rhs.y;
            return *this;
        }
    };

    Pos operator+(const Pos& lhs, const Pos& rhs)
    {
        return {lhs.x + rhs.x, lhs.y + rhs.y};
    }

    struct Layout {
        LayoutStyle style;
        int width = 0;
        int height = 0;
        Pos offset = {0, 0};
        Pos orig = {0, 0};
    };

    struct Panel {
        std::string name;
        Pos pos = {0, 0};
        int padding = 4;
        int headerHeight = 32;
    };

    union Color
    {
        uint32_t hex;

        struct { unsigned char a: 8, b: 8, g: 8, r: 8; };
    };

    glm::vec4 toVec4(const Color& c)
    {
        return { c.r / 255.f, c.g / 255.f, c.b / 255.f, c.a / 255.f };
    }

    struct Palette {
        Color e100;
        Color e200;
        Color e300;
        Color e400;
        Color e500;
        Color e600;
        Color e700;
        Color e800;
        Color e900;
    };

    Palette greyPalette {
        .e100 { 0xf3f4f8ff },
        .e200 { 0xd2d4daff },
        .e300 { 0xb3b5bdff },
        .e400 { 0x9496a1ff },
        .e500 { 0x777986ff },
        .e600 { 0x5b5d6bff },
        .e700 { 0x404252ff },
        .e800 { 0x282a3aff },
        .e900 { 0x101223ff }
    };

    struct Theme {
        Color panelColor;
        Color buttonColor;
        Color buttonHoverColor;
        Color buttonPressedColor;
    };

    Theme lavender {
        .panelColor { 0x28282bff },
        .buttonColor { 0xa973d9ff },
        .buttonHoverColor { 0x9355c8ff },
        .buttonPressedColor { 0x7e42aeff }
    };

    Theme grey {
        .panelColor { greyPalette.e700 },
        .buttonColor { greyPalette.e300 },
        .buttonHoverColor { greyPalette.e400 },
        .buttonPressedColor { greyPalette.e500 }
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
        std::queue<Layout> layoutStack;
        std::string draggedPanel;
        std::string currentPanel;
        Theme theme = grey;
        int zOrder = 0;
    };
    Context context;

    struct Region {
        int x;
        int y;
        int width;
        int height;
    };

    bool inBetween(int value, int min, int max)
    {
        return min <= value && value < max;
    }

    bool inRegion(const Pos& pos, const Region& region)
    {
        return inBetween(pos.x, region.x, region.x + region.width) && inBetween(pos.y, region.y, region.y + region.height);
    }

    void begin(unsigned int shaderId, RenderData renderData, int mouseX, int mouseY, bool mouseDown)
    {
        context.mouseX = mouseX;
        context.mouseY = mouseY;
        context.mouseDown = mouseDown;
        context.shaderId = shaderId;
        context.renderData = renderData;
        context.zOrder = 0;
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

    void panelBegin(const std::string& name, int x, int y, const LayoutStyle& layoutStyle, int padding)
    {
        if (not context.panels.contains(name))
        {
            Panel panel = { name, x, y, padding };
            context.panels[name] = panel;
        }
        context.currentPanel = name;
        Panel& panel = context.panels[name];
        Layout layout{layoutStyle};
        layout.orig = panel.pos + Pos{ panel.padding, panel.padding + panel.headerHeight };
        context.layoutStack.push(layout);

        Render::setLayer(++context.zOrder);

        if (context.activeElement == name)
        {
            panel.pos += Pos{ context.mouseX - context.previousMouseX, context.mouseY - context.previousMouseY };
        }
    }

    void panelEnd()
    {
        Panel& panel = context.panels[context.currentPanel];
        auto layout = context.layoutStack.front();
        context.layoutStack.pop();
        int x = panel.pos.x;
        int y = panel.pos.y;
        int width = 2*panel.padding + layout.width;
        int height = 2*panel.padding + panel.headerHeight + layout.height;

        Render::Material material;
        material.name = panel.name;
        material.shader = context.shaderId;
        material.renderData = context.renderData;
        material.uniform4fs["color"] = toVec4(context.theme.panelColor);
        // TODO: Add projection and view support to render context
        material.uniformMatrix4fvs["projection"] = glm::ortho(0.f, 1920.f, 1080.f, 0.f, -100.f, 100.f);
        material.uniformMatrix4fvs["view"] = glm::mat4(1.0f);
        Render::setSubLayer(0);
        Render::setMaterial(material);

        Render::queue({ {x, y}, {x + width, y}, {x + width, y + height}, {x + width, y + height}, {x, y + height}, {x, y} });
        context.currentPanel = "";

        bool underMouse = inRegion({context.mouseX, context.mouseY}, {x, y, width, height});
        if (context.activeElement == "" && underMouse && context.mouseDown)
        {
            context.activeElement = panel.name;
        }
    }

    Pos claimSpot(int width, int height)
    {
        Layout& layout = context.layoutStack.front();
        Pos result =  { layout.orig.x + layout.offset.x, layout.orig.y + layout.offset.y };
        layout.width = layout.style.type == LayoutStyle::Row ? layout.offset.x + width : std::max(width, layout.width);
        layout.height = layout.style.type == LayoutStyle::Column ? layout.offset.y + height : std::max(height, layout.height);
        layout.offset += layout.style.type == LayoutStyle::Row ? Pos{ width + layout.style.spacing, 0 } : Pos{ 0, height + layout.style.spacing };
        return result;
    }

    bool button(const std::string& name, int width, int height)
    {
        auto mouseX = context.mouseX;
        auto mouseY = context.mouseY;

        auto [x, y] = claimSpot(width, height);

        bool underMouse = inRegion({mouseX, mouseY}, {x, y, width, height});
        if (underMouse && context.mouseDown && context.activeElement == "")
        {
            context.activeElement = name;
        }
        
        Render::Material material;
        material.name = name;
        material.shader = context.shaderId;
        material.renderData = context.renderData;
        glm::vec4 color = toVec4(context.theme.buttonColor);
        if (underMouse)
        {
            color = toVec4(context.theme.buttonHoverColor);
        }
        if (context.activeElement == name)
        {
            color = toVec4(context.theme.buttonPressedColor);
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
