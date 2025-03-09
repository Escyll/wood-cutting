#ifndef ECS_SYSTEMS_SYSTEMS_H
#define ECS_SYSTEMS_SYSTEMS_H

// TODO: Move ECS and Renderer to library, keep Systems in app

#include <random>

#include "ECS/ECS.h"
#include "Renderer/Renderer.h"

using Color = glm::vec4;
using Pos = glm::vec2;

enum Missions
{
    START,
    GATHER_WOOD,
    WOOD_GATHERED,
    GATHER_CLAY,
    CLAY_GATHERED,
    GATHER_GLAZE,
    GLAZE_GATHERED
};

struct GameState
{
    Missions mission = Missions::START;
    int woodGathered = 0;
    int glazeGathered = 0;
    int clayGathered = 0;
};

struct Patrol
{
    float from;
    float to;
    float speed;
    int direction;
};

struct Tree {};

struct MovementSystem
{
    void run(Registry &registry, float deltaTime)
    {
        auto& pos = registry.get<Pos>(tink);
        glm::vec2 displacement {0.f, 0.f};
        if (isKeyPressed(window, GLFW_KEY_W))
        {
            displacement.y -= 1;
        }
        if (isKeyPressed(window, GLFW_KEY_S))
        {
            displacement.y += 1;
        }
        if (isKeyPressed(window, GLFW_KEY_A))
        {
            displacement.x -= 1;
        }
        if (isKeyPressed(window, GLFW_KEY_D))
        {
            displacement.x += 1;
        }
        if (glm::length(displacement) > 0.1f)
        {
            pos += speed * deltaTime * glm::normalize(displacement);
        }
    }
    float speed = 150.f;
    GLFWwindow* window;
    Entity tink;
};

struct WoodGatheringSystem
{
    void run(Registry &registry, float deltaTime)
    {
        auto& tinkPos = registry.get<Pos>(tink);
        for (auto [treeEntity, _]: registry.each<Tree>())
        {
            auto& treePos = registry.get<Pos>(treeEntity);
            if (glm::length(tinkPos - treePos) < 15.f)
            {
                registry.remove(treeEntity);
                gameState.woodGathered++;
            }
        }
    }
    Entity tink;
    GameState& gameState;
};

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
        if (isKeyPressed(window, GLFW_KEY_P))
        {        
            if (registry.has<Patrol>(worker))
            {
                registry.remove<Patrol>(worker);
            }
            else
            {
                registry.insert<Patrol>(worker, Patrol{-0.5f, 0.5f, .5f, 1});
            }
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
    Entity worker;
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

struct Ignore {};
struct RenderSystem
{
    void run(Registry &registry, float deltaTime)
    {
        glUseProgram(shaderID);
        for (auto [entity, position, renderData, color] : registry.each<glm::vec2, RenderData, glm::vec4>())
        {
            if (registry.has<Ignore>(entity))
                continue;
            auto model = glm::translate(glm::mat4(1.0f), glm::vec3(position.x, position.y, 0.f));
            setUniform(shaderID, "model", model);
            auto projection = glm::ortho(0.f, 1920.f, 1080.f, 0.f);
            //projection = glm::scale(projection, glm::vec3(1.f, -1.f, 1.f));
            //projection = glm::translate(projection, glm::vec3(
            setUniform(shaderID, "projection", projection);
            setUniform(shaderID, "color", color);
            render(renderData);
        }
    }
    unsigned int shaderID;
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

namespace {
std::random_device rd;
std::mt19937 gen(rd());
std::uniform_real_distribution<> dis(-1.0, 1.0);
glm::vec2 getRandomPos()
{
    return {dis(gen), dis(gen)};
}
}

struct TreePlantingSystem
{
    void run(Registry &registry, float deltaTime)
    {
        if (isKeyPressed(window, GLFW_KEY_P))
        {
            for (int i = 0; i < 1000; i++)
            {
                pressed = true;
                auto tree = registry.create();
                registry.insert<Tree>(tree, {});
                registry.insert<glm::vec2>(tree, getRandomPos());
                registry.insert<RenderData>(tree, {circleVAO, circleElementCount, GL_TRIANGLE_FAN});             
                registry.insert<glm::vec4>(tree, {0.2, 1.0, 0.2, 1.0});
            }
            std::cout << registry.getEntities<Tree>().size() << " trees planted" << std::endl;
        }

        if (not isKeyPressed(window, GLFW_KEY_P))
        {
            pressed = false;
        }
    }
    GLFWwindow* window;
    unsigned int circleVAO;
    unsigned int circleElementCount;
    bool pressed = false;
};

#endif
