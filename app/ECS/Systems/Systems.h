#ifndef ECS_SYSTEMS_SYSTEMS_H
#define ECS_SYSTEMS_SYSTEMS_H

// TODO: Move ECS and Renderer to library, keep Systems in app

#include <random>

#include "ECS/ECS.h"
#include "Renderer/Renderer.h"

using Color = glm::vec4;
using Pos = glm::vec2;

enum class TileType
{
    UNSET = 0,
    WATER,
    GRASS,
    PATH,
    PATH_MINERAL_1,
    PATH_MINERAL_2,
    PATH_MINERAL_3,
    GRASS_WATER_N,
    GRASS_WATER_NE,
    GRASS_WATER_E,
    GRASS_WATER_SE,
    GRASS_WATER_S,
    GRASS_WATER_SW,
    GRASS_WATER_W,
    GRASS_WATER_NW,
    GRASS_PATH_N,
    GRASS_PATH_NE,
    GRASS_PATH_E,
    GRASS_PATH_SE,
    GRASS_PATH_S,
    GRASS_PATH_SW,
    GRASS_PATH_W,
    GRASS_PATH_NW,
    PATH_GRASS_N,
    PATH_GRASS_NE,
    PATH_GRASS_E,
    PATH_GRASS_SE,
    PATH_GRASS_S,
    PATH_GRASS_SW,
    PATH_GRASS_W,
    PATH_GRASS_NW,
    PATH_WATER_N,
    PATH_WATER_NE,
    PATH_WATER_E,
    PATH_WATER_SE,
    PATH_WATER_S,
    PATH_WATER_SW,
    PATH_WATER_W,
    PATH_WATER_NW,
    WATER_GRASS_N,
    WATER_GRASS_NE,
    WATER_GRASS_E,
    WATER_GRASS_SE,
    WATER_GRASS_S,
    WATER_GRASS_SW,
    WATER_GRASS_W,
    WATER_GRASS_NW,
    WATER_PATH_N,
    WATER_PATH_NE,
    WATER_PATH_E,
    WATER_PATH_SE,
    WATER_PATH_S,
    WATER_PATH_SW,
    WATER_PATH_W,
    WATER_PATH_NW,
};

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

bool editing = false;

struct MovementSystem
{
    void run(Registry &registry, float deltaTime)
    {
        if (editing)
            return;
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
    float speed = 200.f;
    GLFWwindow* window;
    Entity tink;
};

struct TileSystem
{
    void selectTile(const glm::ivec2& nextSelectedPosition, Registry &registry)
    {
        int nextSelectedTile = 0;
        for (auto [tileEntity, pos, _]: registry.each<glm::ivec2, TileType>())
        {
            if (pos.x == nextSelectedPosition.x && pos.y == nextSelectedPosition.y)
            {
                nextSelectedTile = tileEntity;
            }
        }
        if (nextSelectedTile != 0)
        {
            selectedTile = nextSelectedTile;
            selectedPosition = nextSelectedPosition;
        }
    }

    void run(Registry &registry, float deltaTime)
    {
        if (isKeyPressed(window, GLFW_KEY_RIGHT) && keyPressed == 0 && editing)
        {
            keyPressed = GLFW_KEY_RIGHT;
            selectTile({selectedPosition.x + 1, selectedPosition.y}, registry);
        }
        else if (isKeyPressed(window, GLFW_KEY_LEFT) && keyPressed == 0 && editing)
        {
            keyPressed = GLFW_KEY_LEFT;
            selectTile({selectedPosition.x - 1, selectedPosition.y}, registry);
        }
        else if (isKeyPressed(window, GLFW_KEY_UP) && keyPressed == 0 && editing)
        {
            keyPressed = GLFW_KEY_UP;
            selectTile({selectedPosition.x, selectedPosition.y - 1}, registry);
        }
        else if (isKeyPressed(window, GLFW_KEY_DOWN) && keyPressed == 0 && editing)
        {
            keyPressed = GLFW_KEY_DOWN;
            selectTile({selectedPosition.x, selectedPosition.y + 1}, registry);
        }
        else if (isKeyPressed(window, GLFW_KEY_E) && keyPressed == 0)
        {
            keyPressed = GLFW_KEY_E;
            editing = !editing;
            commandString = "";
        }
        else if (isKeyPressed(window, GLFW_KEY_S) && keyPressed == 0 && editing && commandString.empty())
        {
            keyPressed = GLFW_KEY_S;
            std::ofstream wf("level.dat", std::ios::out | std::ios::binary);
            auto tiles = registry.each<glm::ivec2, TileType>();
            uint32_t count = tiles.size();
            std::cerr << "Writing count " << count << std::endl;
            wf.write(reinterpret_cast<const char*>(&count), sizeof(count));
            for (auto [tileEntity, pos, tileType]: tiles)
            {
                wf.write(reinterpret_cast<char*>(&pos), sizeof(pos));
                wf.write(reinterpret_cast<char*>(&tileType), sizeof(tileType));
            }
            wf.close();
            std::cerr << "Saved file" << std::endl;
        }
        else if (isKeyPressed(window, GLFW_KEY_L) && keyPressed == 0 && editing)
        {
            keyPressed = GLFW_KEY_L;
            std::ifstream wf("level.dat", std::ios::in | std::ios::binary);
            uint32_t count = 0;
            std::cerr << "Reading count ";
            wf.read(reinterpret_cast<char*>(&count), sizeof(count));
            if (count == 0)
            {
                return;
            }
            for (auto [tileEntity, tileType]: registry.each<TileType>())
            {
                registry.remove(tileEntity);
            }
            for (uint32_t i = 0; i < count; i++)
            {
                glm::ivec2 pos;
                TileType type;
                wf.read(reinterpret_cast<char*>(&pos), sizeof(pos));
                wf.read(reinterpret_cast<char*>(&type), sizeof(type));
                auto tile = registry.create();
                registry.insert<glm::ivec2>(tile, pos);
                registry.insert<TileType>(tile, type);
            }
        }
        else if (isKeyPressed(window, GLFW_KEY_G) && keyPressed == 0 && editing)
        {
            keyPressed = GLFW_KEY_G;
            registry.replace<TileType>(selectedTile, TileType::GRASS);
        }
        if (!isKeyPressed(window, keyPressed))
        {
            keyPressed = 0;
        }

        auto projection = glm::ortho(0.f, 30*1920.f/1080.f, 30.f, 0.f);
        setUniform(shaderID, "projection", projection);

        glEnable(GL_DEPTH_TEST);
        for (auto [tileEntity, pos, type]: registry.each<glm::ivec2, TileType>())
        {
            auto model = glm::translate(glm::mat4(1.0f), glm::vec3(pos.x - 0.5f, pos.y - 0.5f, selectedTile == tileEntity ? 1.f : 0.f));
            setUniform(shaderID, "model", model);
            if (editing)
            {
                if (selectedTile == 0)
                {
                    selectedTile = tileEntity;
                    selectedPosition = pos;
                }
                setUniform(shaderID, "color", selectedTile == tileEntity ? glm::vec4{1.f, 1.f, 0.f, 1.f} : type == TileType::GRASS ? glm::vec4{0.f, 1.f, 0.f, 1.f} : glm::vec4{0.f, 0.f, 0.f, 1.f});
                render(lineRenderData);
            }
        }
        glDisable(GL_DEPTH_TEST);
    }
    unsigned int shaderID;
    GLFWwindow* window;
    unsigned int keyPressed = 0;
    Entity selectedTile = 0;
    glm::ivec2 selectedPosition {-1, -1};
    RenderData lineRenderData;
    std::string commandString;
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
