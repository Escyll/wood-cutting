#include "Imgui/Imgui.h"

#include <iostream>
#include <string>
#include <unordered_map>
#include <deque>
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

    Pos operator-(const Pos& lhs, const Pos& rhs)
    {
        return Pos{lhs.x - rhs.x, lhs.y - rhs.y};
    }

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
        Pos mousePos;
        bool mouseDown;
        Pos prevMousePos;
        std::string activeElement;
        unsigned int shaderId;
        unsigned int textureShaderId;
        RenderData renderData;
        std::unordered_map<std::string, Panel> panels;
        std::deque<Layout> layoutStack;
        std::string currentPanel;
        Theme theme = grey;
        int zOrder = 0;
        Render::Camera* camera = nullptr;
        GLFWcursorposfun prevCursorposCallback = nullptr;
        GLFWmousebuttonfun prevMousebuttonCallback = nullptr;
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

    void begin(unsigned int shaderId, unsigned int textureShaderId, RenderData renderData, Render::Camera* camera)
    {
        context.shaderId = shaderId;
        context.textureShaderId = textureShaderId;
        context.renderData = renderData;
        context.zOrder = 0;
        context.camera = camera;
        Render::flush();
        Render::printGLDebug("Rendering UI");
        Render::setCamera(context.camera);
    }
    
    void end()
    {
        if (!context.mouseDown)
        {
            context.activeElement = "";
        }
        context.prevMousePos = context.mousePos;
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
        context.layoutStack.push_back(layout);

        Render::setLayer(++context.zOrder);

        if (context.activeElement == name)
        {
            panel.pos += context.mousePos - context.prevMousePos;
        }
    }

    void panelEnd()
    {
        Panel& panel = context.panels[context.currentPanel];
        auto layout = context.layoutStack.back();
        context.layoutStack.pop_back();
        int x = panel.pos.x;
        int y = panel.pos.y;
        int width = 2*panel.padding + layout.width;
        int height = 2*panel.padding + panel.headerHeight + layout.height;

        Render::Material material;
        material.name = panel.name;
        material.shader = context.shaderId;
        material.renderData = context.renderData;
        material.uniform4fs["color"] = toVec4(context.theme.panelColor);
        Render::setSubLayer(0);
        Render::setMaterial(material);

        Render::queue({ {x, y}, {x + width, y}, {x + width, y + height}, {x + width, y + height}, {x, y + height}, {x, y} });

        context.currentPanel = "";

        bool underMouse = inRegion(context.mousePos, {x, y, width, height});
        if (context.activeElement == "" && underMouse && context.mouseDown)
        {
            context.activeElement = panel.name;
        }
    }

    Pos claimSpot(int width, int height)
    {
        Layout& layout = context.layoutStack.back();
        Pos result =  { layout.orig.x + layout.offset.x, layout.orig.y + layout.offset.y };
        layout.width = layout.style.type == LayoutStyle::Row ? layout.offset.x + width : std::max(width, layout.width);
        layout.height = layout.style.type == LayoutStyle::Column ? layout.offset.y + height : std::max(height, layout.height);
        layout.offset += layout.style.type == LayoutStyle::Row ? Pos{ width + layout.style.spacing, 0 } : Pos{ 0, height + layout.style.spacing };
        return result;
    }

    void beginLayout(const LayoutStyle& layoutStyle)
    {
        Layout& parentLayout = context.layoutStack.back();
        Layout newLayout{layoutStyle};
        newLayout.orig = parentLayout.orig + parentLayout.offset;
        context.layoutStack.push_back(newLayout);
    }

    void endLayout()
    {
        Layout currentLayout = context.layoutStack.back();
        context.layoutStack.pop_back();
        claimSpot(currentLayout.width, currentLayout.height);
    }

    std::vector<glm::vec2> generateTriangulation(const glm::vec2& bottomLeft, const glm::vec2& size)
    {
        return {
            { bottomLeft },
            { bottomLeft + glm::vec2{ size.x, 0 } },
            { bottomLeft + glm::vec2{ size.x, size.y } },
            { bottomLeft + glm::vec2{ size.x, size.y } },
            { bottomLeft + glm::vec2{ 0, size.y } },
            { bottomLeft }
        };
    }

    std::vector<glm::vec2> generateTriangulationUVs(const glm::vec2& bottomLeft, const glm::vec2& size)
    {
        return {
            { bottomLeft + glm::vec2{ 0, size.y } },
            { bottomLeft + size },
            { bottomLeft + glm::vec2{ size.x, 0 } },
            { bottomLeft + glm::vec2{ size.x, 0 } },
            { bottomLeft },
            { bottomLeft + glm::vec2{ 0, size.y } }
        };
    }

    bool button(const std::string& name, int width, int height)
    {
        auto mousePos = context.mousePos;

        auto [x, y] = claimSpot(width, height);

        bool underMouse = inRegion(mousePos, {x, y, width, height});
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
        Render::setSubLayer(1);
        Render::setMaterial(material);
        auto positions = generateTriangulation({x, y}, {width, height});
        Render::queue(positions);

        return context.activeElement == name && underMouse && !context.mouseDown;
    }

    bool imageButton(const std::string& name, unsigned int image, const Frame& frame, int width, int height)
    {
        auto mousePos = context.mousePos;

        auto [x, y] = claimSpot(width, height);

        bool underMouse = inRegion(mousePos, {x, y, width, height});
        if (underMouse && context.mouseDown && context.activeElement == "")
        {
            context.activeElement = name;
        }
        
        Render::Material material;
        material.name = name;
        material.shader = context.textureShaderId;
        material.renderData = context.renderData;
        material.texture = image;
        Render::setSubLayer(1);
        Render::setMaterial(material);

        auto positions = generateTriangulation({x, y}, {width, height});
        auto texCoords = generateTriangulationUVs(frame.textureRegion.bottomLeft, frame.textureRegion.size);
        Render::queue(positions, texCoords);

        return context.activeElement == name && underMouse && !context.mouseDown;
    }

    void cursorPosCallback(GLFWwindow* window, double xpos, double ypos)
    {
        context.mousePos = Pos{ static_cast<int>(xpos), static_cast<int>(ypos)};
        if (context.prevCursorposCallback)
        {
            context.prevCursorposCallback(window, xpos, ypos);
        }
    }

    void mousebuttonCallback(GLFWwindow* window, int button, int action, int mods)
    {
        context.mouseDown = button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS;
        if (context.prevMousebuttonCallback)
        {
            context.prevMousebuttonCallback(window, button, action, mods);
        }
    }

    void installCallbacks(GLFWwindow* window)
    {
        context.prevCursorposCallback = glfwSetCursorPosCallback(window, cursorPosCallback);
        context.prevMousebuttonCallback = glfwSetMouseButtonCallback(window, mousebuttonCallback);
    }
}
