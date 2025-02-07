#ifndef ECS_SYSTEMS_SYSTEMS_H
#define ECS_SYSTEMS_SYSTEMS_H

// TODO: Move ECS and Renderer to library, keep Systems in app
#include "ECS/ECS.h"
#include "Renderer/Renderer.h"

struct PongMovementSystem
{
    void run(Registry &registry, float deltaTime)
    {
        if (isKeyPressed(window, GLFW_KEY_W))
        {
            auto& pos = registry.get<glm::vec2>(bar1);
            pos.y += speed*deltaTime;
        }
        if (isKeyPressed(window, GLFW_KEY_S))
        {
            auto& pos = registry.get<glm::vec2>(bar1);
            pos.y -= speed*deltaTime;
        }
        if (isKeyPressed(window, GLFW_KEY_UP))
        {
            auto& pos = registry.get<glm::vec2>(bar2);
            pos.y += speed*deltaTime;
        }
        if (isKeyPressed(window, GLFW_KEY_DOWN))
        {
            auto& pos = registry.get<glm::vec2>(bar2);
            pos.y -= speed*deltaTime;
        }
        for (auto bar : {bar1, bar2})
        {
            auto& pos = registry.get<glm::vec2>(bar);
            pos.y = std::max(-1.f, std::min(0.8f, pos.y));
        }
    }
    Entity bar1;
    Entity bar2;
    float speed = 0.8f;
    GLFWwindow* window;
};

struct Direction : public glm::vec2
{
    Direction(double x, double y) : glm::vec2(x, y) {}
};

struct PongCollisionSystem
{
    void run(Registry& registry, float deltaTime)
    {
        auto& posBar1 = registry.get<glm::vec2>(bar1);
        auto& posBar2 = registry.get<glm::vec2>(bar2);
        auto& posBall = registry.get<glm::vec2>(ball);
        auto& colBar1 = registry.get<glm::vec4>(bar1);
        auto& colBar2 = registry.get<glm::vec4>(bar2);

        auto& dirBall = registry.get<Direction>(ball);
        posBall += speed*deltaTime*0.5f*dirBall;
        if (posBall.x <= -0.99f || posBall.x >= 0.99f)
        {
            dirBall.x = -dirBall.x;
        }
        if (posBall.y <= -0.99f || posBall.y >= 0.99f)
        {
            dirBall.y = -dirBall.y;
        }
        if (std::abs(posBall.x + 0.01f - (posBar1.x + 0.01f)) < 0.01f)
        {
            if (posBar1.y < posBall.y && posBall.y < posBar1.y + 0.2f)
            {
                dirBall.x = -dirBall.x;
                speed *= 1.01;
            }
        }
        if (std::abs(posBall.x - 0.01f - (posBar2.x - 0.01f)) < 0.01f)
        {
            if (posBar2.y < posBall.y && posBall.y < posBar2.y + 0.2f)
            {
                dirBall.x = -dirBall.x;
                speed *= 1.01;
            }
        }
    }
    Entity bar1;
    Entity bar2;
    Entity ball;
    float speed = 1.0f;
};

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
    unsigned int shaderID;
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
