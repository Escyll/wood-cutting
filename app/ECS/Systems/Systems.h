#ifndef ECS_SYSTEMS_SYSTEMS_H
#define ECS_SYSTEMS_SYSTEMS_H

//TODO: Move ECS and Renderer to library, keep Systems in app
#include "ECS/ECS.h"
#include "Renderer/Renderer.h"

struct RenderSystem
{
    void run(Registry& registry, float deltaTime)
    {
        auto& position = registry.get<glm::vec2>(worker);
        auto& renderData = registry.get<RenderData>(worker);
        auto& color = registry.get<glm::vec4>(worker);
        auto trans = glm::translate(glm::mat4(1.0f), glm::vec3(position.x, position.y, 0.f));
        setUniform(shaderID, "transform", trans);
        setUniform(shaderID, "color", color);
        render(renderData);
    }
    Entity worker;
    int shaderID;
};

struct WoodCuttingSystem
{
    void run(Registry& registry, float deltaTime)
    {
        auto& position = registry.get<glm::vec2>(worker);
        position.x += direction*deltaTime*0.5f;

        if (position.x > 0.5f)
        {
            direction *= -1.f;
            position.x = 0.5f - (position.x - 0.5f);
        }

        if (position.x < -0.5f)
        {
            direction *= -1.f;
            position.x = -0.5f + (position.x + 0.5f);
        }
    }

    float direction = 1.f;
    Entity worker;
};

#endif
