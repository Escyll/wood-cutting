#ifndef ECS_SYSTEMS_SYSTEMS_H
#define ECS_SYSTEMS_SYSTEMS_H

// TODO: Move ECS and Renderer to library, keep Systems in app
#include "ECS/ECS.h"
#include "Renderer/Renderer.h"

struct RenderSystem
{
    void run(Registry &registry, float deltaTime)
    {
        for (auto [entity, position, renderData, color] : registry.each<glm::vec2, RenderData, glm::vec4>())
        {
            auto trans = glm::translate(glm::mat4(1.0f), glm::vec3(position.x, position.y, 0.f));
            setUniform(shaderID, "transform", trans);
            setUniform(shaderID, "color", color);
            render(renderData);
        }
    }
    int shaderID;
};

struct Patrol
{
    float from;
    float to;
    float speed;
    int direction;
};

struct WoodCuttingSystem
{
    void run(Registry &registry, float deltaTime)
    {
        for (auto [entity, position, patrol] : registry.each<glm::vec2, Patrol>())
        {
            position.x += patrol.speed * patrol.direction * deltaTime;

            if (position.x > patrol.to)
            {
                patrol.direction = -1;
                position.x = patrol.to - (position.x - patrol.to);
            }

            if (position.x < patrol.from)
            {
                patrol.direction = 1;
                position.x = -patrol.from + (position.x + patrol.from);
            }
        }
    }
};

#endif
