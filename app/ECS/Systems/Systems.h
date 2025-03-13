#ifndef ECS_SYSTEMS_SYSTEMS_H
#define ECS_SYSTEMS_SYSTEMS_H

// TODO: Move ECS and Renderer to library, keep Systems in app

#include <random>
#include <sstream>

#include "ECS/ECS.h"
#include "ECS/Systems/InputSystem.h"
#include "Renderer/Renderer.h"
#include "Catalog.h"

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
    CLAY,
};

enum class DecoType
{
    WOOD = 0,
    GLAZE,
    FLOWER,
    OVEN,
    BRIDGE_HOR,
    BRIDGE_VER
};

enum Missions
{
    START,
    CHAT_GEORGE_1,
    CHAT_GEORGE_2,
    CHAT_GEORGE_3,
    GATHER_WOOD,
    CHAT_GEORGE_4,
    CHAT_GEORGE_5,
    GATHER_CLAY,
    CHAT_GEORGE_6,
    BAKE_PIECE,
    BAKE_PIECE_HAND_IN,
    CHAT_GEORGE_7,
    CHAT_GEORGE_8,
    GATHER_WOOD_AND_GLAZE,
    CHAT_GEORGE_9,
    GLAZE_PIECE,
    GLAZE_PIECE_HAND_IN,
    SHOW_GEORGE,
    CHAT_GEORGE_JURY,
    CHAT_GEORGE_YOU_WON,
    END
};

struct GameState
{
    Missions mission = Missions::START;
    int woodGathered = 0;
    int glazeGathered = 0;
    int clayGathered = 0;
    bool allowMovement = true;
};

struct Patrol
{
    float from;
    float to;
    float speed;
    int direction;
};

struct Tree {};

struct Blocked{};

bool editing = false;

struct MovementSystem
{
    void run(Registry &registry, float deltaTime)
    {
        if (editing || !gameState.allowMovement)
            return;
        auto& pos = registry.get<Pos>(tink);
        glm::vec2 displacement {0.f, 0.f};
        if (isHolded(GLFW_KEY_W))
        {
            displacement.y -= 1;
        }
        if (isHolded(GLFW_KEY_S))
        {
            displacement.y += 1;
        }
        if (isHolded(GLFW_KEY_A))
        {
            displacement.x -= 1;
        }
        if (isHolded(GLFW_KEY_D))
        {
            displacement.x += 1;
        }
        if (glm::length(displacement) > 0.1f)
        {
            auto newPos = pos + speed * deltaTime * glm::normalize(displacement);
            glm::ivec2 tilePos { newPos.x, newPos.y };
            for (auto [_, pos, __] : registry.each<glm::ivec2, Blocked>())
            {
                if (pos == tilePos)
                    return;
            }
            pos = newPos;
        }
    }
    GameState& gameState;
    float speed = 4.f;
    Entity tink;
};

void loadLevel(Registry& registry)
{
    std::cerr << "Loading level" << std::endl;
    std::ifstream wf("assets/levels/level.dat", std::ios::in | std::ios::binary);
    uint32_t count = 0;
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
    wf.read(reinterpret_cast<char*>(&count), sizeof(count));
    if (count == 0)
    {
        return;
    }
    for (auto [tileEntity, tileType]: registry.each<DecoType>())
    {
        registry.remove(tileEntity);
    }
    for (uint32_t i = 0; i < count; i++)
    {
        glm::ivec2 pos;
        DecoType type;
        wf.read(reinterpret_cast<char*>(&pos), sizeof(pos));
        wf.read(reinterpret_cast<char*>(&type), sizeof(type));
        auto tile = registry.create();
        registry.insert<glm::ivec2>(tile, pos);
        registry.insert<DecoType>(tile, type);
    }
    wf.read(reinterpret_cast<char*>(&count), sizeof(count));
    if (count == 0)
    {
        return;
    }
    for (auto [tileEntity, _]: registry.each<Blocked>())
    {
        registry.remove(tileEntity);
    }
    for (uint32_t i = 0; i < count; i++)
    {
        glm::ivec2 pos;
        wf.read(reinterpret_cast<char*>(&pos), sizeof(pos));
        for (auto [tile, tilePos, __]: registry.each<glm::ivec2, TileType>())
        {
            if (pos == tilePos)
            {
                registry.insert<glm::ivec2>(tile, pos);
                registry.insert<Blocked>(tile, {});
            }
        }
    }

    std::cerr << "Level loaded" << std::endl;
}

struct TileSystem
{
    std::array<glm::vec2, 4> toTextureCoord(const glm::ivec2& tilePos, const glm::ivec2 tileCount, const glm::ivec2& span = {1, 1})
    {
        return {
            glm::vec2{(float) tilePos.x / tileCount.x, (float) tilePos.y / tileCount.y},
            glm::vec2{((float) tilePos.x + span.x) / tileCount.x, (float) tilePos.y / tileCount.y},
            glm::vec2{((float) tilePos.x + span.x) / tileCount.x, ((float) tilePos.y + span.y) / tileCount.y},
            glm::vec2{(float) tilePos.x / tileCount.x, ((float) tilePos.y + span.y) / tileCount.y}
        };
    }

    void selectTile(const glm::ivec2& nextSelectedPosition, Registry &registry)
    {
        int nextSelectedTile = 0;
        TileType nextSelectedType = TileType::UNSET;
        for (auto [tileEntity, pos, type]: registry.each<glm::ivec2, TileType>())
        {
            if (pos.x == nextSelectedPosition.x && pos.y == nextSelectedPosition.y)
            {
                nextSelectedTile = tileEntity;
                nextSelectedType = type;
            }
        }
        if (nextSelectedTile != 0)
        {
            selectedTile = nextSelectedTile;
            selectedPosition = nextSelectedPosition;
            selectedTileType = nextSelectedType;
        }
    }

    TileType typeOfNeighbor(const glm::ivec2& neighbour, Registry& registry)
    {
        auto neighbourPos = selectedPosition + neighbour;
        for (auto [_, pos, tileType]: registry.each<glm::ivec2, TileType>())
        {
            if (pos == neighbourPos)
            {
                return tileType;
            }
        }
        return TileType::UNSET;
    }

    void run(Registry &registry, float deltaTime)
    {
        if (isPressedOrRepeated(GLFW_KEY_RIGHT) && editing)
        {
            selectTile({selectedPosition.x + 1, selectedPosition.y}, registry);
        }
        else if (isPressedOrRepeated(GLFW_KEY_LEFT) && editing)
        {
            selectTile({selectedPosition.x - 1, selectedPosition.y}, registry);
        }
        else if (isPressedOrRepeated(GLFW_KEY_UP) && editing)
        {
            selectTile({selectedPosition.x, selectedPosition.y - 1}, registry);
        }
        else if (isPressedOrRepeated(GLFW_KEY_DOWN) && editing)
        {
            selectTile({selectedPosition.x, selectedPosition.y + 1}, registry);
        }
        else if (isPressed(GLFW_KEY_E))
        {
            editing = !editing;
        }
        else if (isPressed(GLFW_KEY_S) && editing)
        {
            std::cerr << "Saving level" << std::endl;
            std::ofstream wf("assets/levels/level.dat", std::ios::out | std::ios::binary);
            auto tiles = registry.each<glm::ivec2, TileType>();
            uint32_t count = tiles.size();
            wf.write(reinterpret_cast<const char*>(&count), sizeof(count));
            for (auto [tileEntity, pos, tileType]: tiles)
            {
                wf.write(reinterpret_cast<char*>(&pos), sizeof(pos));
                wf.write(reinterpret_cast<char*>(&tileType), sizeof(tileType));
            }
            auto decoTiles = registry.each<glm::ivec2, DecoType>();
            count = decoTiles.size();
            wf.write(reinterpret_cast<const char*>(&count), sizeof(count));
            for (auto [tileEntity, pos, decoType]: decoTiles)
            {
                wf.write(reinterpret_cast<char*>(&pos), sizeof(pos));
                wf.write(reinterpret_cast<char*>(&decoType), sizeof(decoType));
            }
            auto blockedTiles = registry.each<glm::ivec2, Blocked>();
            count = blockedTiles.size();
            wf.write(reinterpret_cast<const char*>(&count), sizeof(count));
            for (auto [tileEntity, pos, _]: blockedTiles)
            {
                wf.write(reinterpret_cast<char*>(&pos), sizeof(pos));
            }
            wf.close();
            std::cerr << "Level saved" << std::endl;
        }
        else if (isPressed(GLFW_KEY_L) && editing)
        {
            loadLevel(registry);
        }
        else if (isPressed(GLFW_KEY_F2))
        {
            loadLevel(registry);
            gameState = GameState{};
            registry.replace<Pos>(tink, {25, 20});
        }
        else if (isPressed(GLFW_KEY_F1) && editing)
        {
            for (auto [tileEntity, _]: registry.each<TileType>())
            {
                registry.replace<TileType>(tileEntity, TileType::GRASS);
            }
        }
        else if (isPressed(GLFW_KEY_G) && editing)
        {
            registry.replace<TileType>(selectedTile, TileType::GRASS);
        }
        else if (isPressed(GLFW_KEY_W) && editing)
        {
            registry.replace<TileType>(selectedTile, TileType::WATER);
        }
        else if (isPressed(GLFW_KEY_P) && editing)
        {
            registry.replace<TileType>(selectedTile, TileType::PATH);;
        }
        else if (isPressed(GLFW_KEY_C) && editing)
        {
            registry.replace<TileType>(selectedTile, TileType::CLAY);
        }
        else if (isPressed(GLFW_KEY_B) && editing)
        {
            bool shifted = isPressed(GLFW_KEY_LEFT_SHIFT) || isPressed(GLFW_KEY_RIGHT_SHIFT);
            if (shifted)
                registry.remove<Blocked>(selectedTile);
            else
                registry.insert_or_replace<Blocked>(selectedTile, {});
        }
        else if (isPressed(GLFW_KEY_1) && editing)
        {
            registry.insert_or_replace<DecoType>(selectedTile, DecoType::WOOD);
        }
        else if (isPressed(GLFW_KEY_2) && editing)
        {
            registry.insert_or_replace<DecoType>(selectedTile, DecoType::GLAZE);
        }
        else if (isPressed(GLFW_KEY_3) && editing)
        {
            registry.insert_or_replace<DecoType>(selectedTile, DecoType::FLOWER);
        }
        else if (isPressed(GLFW_KEY_4) && editing)
        {
            registry.insert_or_replace<DecoType>(selectedTile, DecoType::OVEN);
        }
        else if (isPressed(GLFW_KEY_5) && editing)
        {
            registry.insert_or_replace<DecoType>(selectedTile, DecoType::BRIDGE_HOR);
        }
        else if (isPressed(GLFW_KEY_6) && editing)
        {
            registry.insert_or_replace<DecoType>(selectedTile, DecoType::BRIDGE_VER);
        }
        else if (isPressed(GLFW_KEY_N) && editing)
        {
            auto neighbour = typeOfNeighbor({-1, 1}, registry);
            bool shifted = isPressed(GLFW_KEY_LEFT_SHIFT) || isPressed(GLFW_KEY_RIGHT_SHIFT);
            if (selectedTileType == TileType::GRASS && neighbour == TileType::WATER)
            {
                registry.replace<TileType>(selectedTile, shifted ? TileType::WATER_GRASS_NE : TileType::GRASS_WATER_SW);
            }
            if (selectedTileType == TileType::PATH && neighbour == TileType::WATER)
            {
                registry.replace<TileType>(selectedTile, shifted ? TileType::WATER_PATH_NE : TileType::PATH_WATER_SW);
            }
            if (selectedTileType == TileType::GRASS && neighbour == TileType::PATH)
            {
                registry.replace<TileType>(selectedTile, shifted ? TileType::PATH_GRASS_NE : TileType::GRASS_PATH_SW);
            }
        }
        else if (isPressed(GLFW_KEY_H) && editing)
        {
            auto neighbour = typeOfNeighbor({-1, 0}, registry);
            if (selectedTileType == TileType::GRASS && neighbour == TileType::WATER)
            {
                registry.replace<TileType>(selectedTile, TileType::GRASS_WATER_W);
            }
            if (selectedTileType == TileType::PATH && neighbour == TileType::WATER)
            {
                registry.replace<TileType>(selectedTile, TileType::PATH_WATER_W);
            }
            if (selectedTileType == TileType::GRASS && neighbour == TileType::PATH)
            {
                registry.replace<TileType>(selectedTile, TileType::GRASS_PATH_W);
            }
        }
        else if (isPressed(GLFW_KEY_Y) && editing)
        {
            auto neighbour = typeOfNeighbor({-1, -1}, registry);
            bool shifted = isPressed(GLFW_KEY_LEFT_SHIFT) || isPressed(GLFW_KEY_RIGHT_SHIFT);
            if (selectedTileType == TileType::GRASS && neighbour == TileType::WATER)
            {
                registry.replace<TileType>(selectedTile, shifted ? TileType::WATER_GRASS_SE : TileType::GRASS_WATER_NW);
            }
            if (selectedTileType == TileType::PATH && neighbour == TileType::WATER)
            {
                registry.replace<TileType>(selectedTile, TileType::PATH_WATER_NW);
            }
        }
        else if (isPressed(GLFW_KEY_U) && editing)
        {
            auto neighbour = typeOfNeighbor({0, -1}, registry);
            if (selectedTileType == TileType::GRASS && neighbour == TileType::WATER)
            {
                registry.replace<TileType>(selectedTile, TileType::GRASS_WATER_N);
            }
            if (selectedTileType == TileType::PATH && neighbour == TileType::WATER)
            {
                registry.replace<TileType>(selectedTile, TileType::PATH_WATER_N);
            }
        }
        else if (isPressed(GLFW_KEY_I) && editing)
        {
            auto neighbour = typeOfNeighbor({1, -1}, registry);
            bool shifted = isPressed(GLFW_KEY_LEFT_SHIFT) || isPressed(GLFW_KEY_RIGHT_SHIFT);
            if (selectedTileType == TileType::GRASS && neighbour == TileType::WATER)
            {
                registry.replace<TileType>(selectedTile, shifted ? TileType::WATER_GRASS_SW : TileType::GRASS_WATER_NE);
            }
            if (selectedTileType == TileType::PATH && neighbour == TileType::WATER)
            {
                registry.replace<TileType>(selectedTile, TileType::PATH_WATER_NE);
            }
        }
        else if (isPressed(GLFW_KEY_K) && editing)
        {
            auto neighbour = typeOfNeighbor({1, 0}, registry);
            if (selectedTileType == TileType::GRASS && neighbour == TileType::WATER)
            {
                registry.replace<TileType>(selectedTile, TileType::GRASS_WATER_E);
            }
            if (selectedTileType == TileType::PATH && neighbour == TileType::WATER)
            {
                registry.replace<TileType>(selectedTile, TileType::PATH_WATER_E);
            }
        }
        else if (isPressed(GLFW_KEY_COMMA) && editing)
        {
            auto neighbour = typeOfNeighbor({1, 1}, registry);
            bool shifted = isPressed(GLFW_KEY_LEFT_SHIFT) || isPressed(GLFW_KEY_RIGHT_SHIFT);
            if (selectedTileType == TileType::GRASS && neighbour == TileType::WATER)
            {
                registry.replace<TileType>(selectedTile, TileType::GRASS_WATER_SE);
            }
            if (selectedTileType == TileType::PATH && neighbour == TileType::WATER)
            {
                registry.replace<TileType>(selectedTile, shifted ? TileType::WATER_PATH_NW : TileType::PATH_WATER_SE);
            }
        }
        else if (isPressed(GLFW_KEY_M) && editing)
        {
            auto neighbour = typeOfNeighbor({0, 1}, registry);
            if (selectedTileType == TileType::GRASS && neighbour == TileType::WATER)
            {
                registry.replace<TileType>(selectedTile, TileType::GRASS_WATER_S);
            }
            if (selectedTileType == TileType::PATH && neighbour == TileType::WATER)
            {
                registry.replace<TileType>(selectedTile, TileType::PATH_WATER_S);
            }
            if (selectedTileType == TileType::GRASS && neighbour == TileType::PATH)
            {
                registry.replace<TileType>(selectedTile, TileType::GRASS_PATH_S);
            }
        }
        // Render tiles
        glUseProgram(unlitTextureShader);
        auto projection = glm::ortho(0.f, 30*1920.f/1080.f, 30.f, 0.f);
        setUniform(unlitTextureShader, "projection", projection);
        setUniform(unlitColorShader, "texture1", 0);
        glActiveTexture(GL_TEXTURE0);
        glBindBuffer(GL_ARRAY_BUFFER, texBuffer);
        glEnable(GL_DEPTH_TEST);
        std::array<glm::vec2, 4> texCoords;
        for (auto [tileEntity, pos, type]: registry.each<glm::ivec2, TileType>())
        {
            auto model = glm::translate(glm::mat4(1.0f), glm::vec3(pos.x - 0.5f, pos.y - 0.5f, -0.8f));
            setUniform(unlitTextureShader, "model", model);
            bool renderTile = true;
            switch (type)
            {
                case TileType::GRASS:
                    texCoords = toTextureCoord({0, 0}, {1, 1});
                    glBindTexture(GL_TEXTURE_2D, getTexture(textureCatalog, "Cute_Fantasy_Free/Tiles/Grass_Middle.png"));
                    break;
                case TileType::WATER:
                    texCoords = toTextureCoord({0, 0}, {1, 1});
                    glBindTexture(GL_TEXTURE_2D, getTexture(textureCatalog, "Cute_Fantasy_Free/Tiles/Water_Middle.png"));
                    break;
                case TileType::PATH:
                    texCoords = toTextureCoord({0, 0}, {1, 1});
                    glBindTexture(GL_TEXTURE_2D, getTexture(textureCatalog, "Cute_Fantasy_Free/Tiles/Path_Middle.png"));
                    break;
                case TileType::CLAY:
                    texCoords = toTextureCoord({2, 5}, {3, 6});
                    glBindTexture(GL_TEXTURE_2D, getTexture(textureCatalog, "Cute_Fantasy_Free/Tiles/Path_Tile.png"));
                    break;
                case TileType::GRASS_WATER_NW:
                    texCoords = toTextureCoord({0, 3}, {3, 6});
                    glBindTexture(GL_TEXTURE_2D, getTexture(textureCatalog, "Cute_Fantasy_Free/Tiles/Water_Tile.png"));
                    break;
                case TileType::GRASS_WATER_N:
                    texCoords = toTextureCoord({1, 2}, {3, 6});
                    glBindTexture(GL_TEXTURE_2D, getTexture(textureCatalog, "Cute_Fantasy_Free/Tiles/Water_Tile.png"));
                    break;
                case TileType::GRASS_WATER_NE:
                    texCoords = toTextureCoord({1, 3}, {3, 6});
                    glBindTexture(GL_TEXTURE_2D, getTexture(textureCatalog, "Cute_Fantasy_Free/Tiles/Water_Tile.png"));
                    break;
                case TileType::GRASS_WATER_E:
                    texCoords = toTextureCoord({0, 1}, {3, 6});
                    glBindTexture(GL_TEXTURE_2D, getTexture(textureCatalog, "Cute_Fantasy_Free/Tiles/Water_Tile.png"));
                    break;
                case TileType::WATER_GRASS_SE:
                    texCoords = toTextureCoord({2, 2}, {3, 6});
                    glBindTexture(GL_TEXTURE_2D, getTexture(textureCatalog, "Cute_Fantasy_Free/Tiles/Water_Tile.png"));
                    break;
                case TileType::WATER_GRASS_SW:
                    texCoords = toTextureCoord({0, 2}, {3, 6});
                    glBindTexture(GL_TEXTURE_2D, getTexture(textureCatalog, "Cute_Fantasy_Free/Tiles/Water_Tile.png"));
                    break;
                case TileType::PATH_WATER_W:
                    texCoords = toTextureCoord({0, 1}, {5, 3});
                    glBindTexture(GL_TEXTURE_2D, getTexture(textureCatalog, "Cute_Fantasy_Free/Tiles/Beach_Tile.png"));
                    break;
                case TileType::PATH_WATER_E:
                    texCoords = toTextureCoord({2, 1}, {5, 3});
                    glBindTexture(GL_TEXTURE_2D, getTexture(textureCatalog, "Cute_Fantasy_Free/Tiles/Beach_Tile.png"));
                    break;
                case TileType::PATH_WATER_SE:
                    texCoords = toTextureCoord({2, 2}, {5, 3});
                    glBindTexture(GL_TEXTURE_2D, getTexture(textureCatalog, "Cute_Fantasy_Free/Tiles/Beach_Tile.png"));
                    break;
                case TileType::PATH_WATER_S:
                    texCoords = toTextureCoord({1, 2}, {5, 3});
                    glBindTexture(GL_TEXTURE_2D, getTexture(textureCatalog, "Cute_Fantasy_Free/Tiles/Beach_Tile.png"));
                    break;
                case TileType::PATH_WATER_SW:
                    texCoords = toTextureCoord({0, 2}, {5, 3});
                    glBindTexture(GL_TEXTURE_2D, getTexture(textureCatalog, "Cute_Fantasy_Free/Tiles/Beach_Tile.png"));
                    break;
                case TileType::WATER_PATH_NE:
                    texCoords = toTextureCoord({4, 0}, {5, 3});
                    glBindTexture(GL_TEXTURE_2D, getTexture(textureCatalog, "Cute_Fantasy_Free/Tiles/Beach_Tile.png"));
                    break;
                case TileType::WATER_PATH_NW:
                    texCoords = toTextureCoord({3, 0}, {5, 3});
                    glBindTexture(GL_TEXTURE_2D, getTexture(textureCatalog, "Cute_Fantasy_Free/Tiles/Beach_Tile.png"));
                    break;
                case TileType::PATH_GRASS_NE:
                    texCoords = toTextureCoord({2, 0}, {3, 6});
                    glBindTexture(GL_TEXTURE_2D, getTexture(textureCatalog, "Cute_Fantasy_Free/Tiles/Path_Tile.png"));
                    break;
                case TileType::GRASS_PATH_W:
                    texCoords = toTextureCoord({2, 1}, {3, 6});
                    glBindTexture(GL_TEXTURE_2D, getTexture(textureCatalog, "Cute_Fantasy_Free/Tiles/Path_Tile.png"));
                    break;
                case TileType::GRASS_PATH_SW:
                    texCoords = toTextureCoord({0, 4}, {3, 6});
                    glBindTexture(GL_TEXTURE_2D, getTexture(textureCatalog, "Cute_Fantasy_Free/Tiles/Path_Tile.png"));
                    break;
                case TileType::GRASS_PATH_S:
                    texCoords = toTextureCoord({1, 0}, {3, 6});
                    glBindTexture(GL_TEXTURE_2D, getTexture(textureCatalog, "Cute_Fantasy_Free/Tiles/Path_Tile.png"));
                    break;
                default:
                    renderTile = false;
                    break;
            }
            if (renderTile)
            {
                glBufferSubData(GL_ARRAY_BUFFER, 0, 4*sizeof(glm::vec2), &texCoords[0]);
                render(tileRenderData);
            }
        }
        for (auto [tileEntity, pos, type]: registry.each<glm::ivec2, DecoType>())
        {
            auto model = glm::translate(glm::mat4(1.0f), glm::vec3(pos.x - 0.5f, pos.y - 0.5f, -0.2f));
            bool renderTile = true;
            switch (type)
            {
                case DecoType::WOOD:
                    model = glm::scale(model, {2, 1, 1});
                    texCoords = toTextureCoord({0, 7}, {7, 12}, {2, 1});
                    glBindTexture(GL_TEXTURE_2D, getTexture(textureCatalog, "Cute_Fantasy_Free/Outdoor decoration/Outdoor_Decor_Free.png"));
                    break;
                case DecoType::GLAZE:
                    texCoords = toTextureCoord({0, 4}, {7, 12});
                    glBindTexture(GL_TEXTURE_2D, getTexture(textureCatalog, "Cute_Fantasy_Free/Outdoor decoration/Outdoor_Decor_Free.png"));
                    break;
                case DecoType::FLOWER:
                    texCoords = toTextureCoord({0, 10}, {7, 12});
                    glBindTexture(GL_TEXTURE_2D, getTexture(textureCatalog, "Cute_Fantasy_Free/Outdoor decoration/Outdoor_Decor_Free.png"));
                    break;
                case DecoType::OVEN:
                    model = glm::scale(model, {3, 3, 1});
                    texCoords = toTextureCoord({0, 0}, {1, 1});
                    glBindTexture(GL_TEXTURE_2D, getTexture(textureCatalog, "Oven.png"));
                    break;
                case DecoType::BRIDGE_HOR:
                    model = glm::translate(model, {-.5f, -1.f, -0.f});
                    model = glm::scale(model, {5, 5, 1});
                    texCoords = toTextureCoord({0, 1}, {9, 4}, {3, 3});
                    glBindTexture(GL_TEXTURE_2D, getTexture(textureCatalog, "Cute_Fantasy_Free/Outdoor decoration/Bridge_Wood.png"));
                    break;
                case DecoType::BRIDGE_VER:
                    model = glm::translate(model, {-1.f, -0.5f, -0.f});
                    model = glm::scale(model, {5, 5, 1});
                    texCoords = toTextureCoord({3, 1}, {9, 4}, {3, 3});
                    glBindTexture(GL_TEXTURE_2D, getTexture(textureCatalog, "Cute_Fantasy_Free/Outdoor decoration/Bridge_Wood.png"));
                    break;
                default:
                    renderTile = false;
                    break;
            }
            if (renderTile)
            {
                setUniform(unlitTextureShader, "model", model);
                glBufferSubData(GL_ARRAY_BUFFER, 0, 4*sizeof(glm::vec2), &texCoords[0]);
                render(tileRenderData);
            }
        }

        {
            auto pos = registry.get<Pos>(george);
            auto model = glm::translate(glm::mat4(1.0f), glm::vec3(pos.x - 1.5f, pos.y - 2.f, -0.19f));
            model = glm::scale(model, {3, 3, 1});
            texCoords = toTextureCoord({2, 0}, {6, 10});
            glBindTexture(GL_TEXTURE_2D, getTexture(textureCatalog, "Cute_Fantasy_Free/Player/Player.png"));
            setUniform(unlitTextureShader, "model", model);
            glBufferSubData(GL_ARRAY_BUFFER, 0, 4*sizeof(glm::vec2), &texCoords[0]);
            render(tileRenderData);
            
            pos = registry.get<Pos>(tink);
            model = glm::translate(glm::mat4(1.0f), glm::vec3(pos.x - 1.5f, pos.y - 2.f, -0.18f));
            model = glm::scale(model, {3, 3, 1});
            texCoords = toTextureCoord({0, 0}, {6, 10});
            glBindTexture(GL_TEXTURE_2D, getTexture(textureCatalog, "Cute_Fantasy_Free/Player/Player.png"));
            setUniform(unlitTextureShader, "model", model);
            glBufferSubData(GL_ARRAY_BUFFER, 0, 4*sizeof(glm::vec2), &texCoords[0]);
            render(tileRenderData);
        }
        glDisable(GL_DEPTH_TEST);

        // Render grid
        glUseProgram(unlitColorShader);
        setUniform(unlitColorShader, "projection", projection);

        glEnable(GL_DEPTH_TEST);
        for (auto [tileEntity, pos, type]: registry.each<glm::ivec2, TileType>())
        {
            auto model = glm::translate(glm::mat4(1.0f), glm::vec3(pos.x - 0.5f, pos.y - 0.5f, selectedTile == tileEntity ? -0.3f : -0.4f));
            setUniform(unlitColorShader, "model", model);

            if (editing)
            {
                if (selectedTile == 0)
                {
                    selectedTile = tileEntity;
                    selectedPosition = pos;
                }
                setUniform(unlitColorShader, "color", selectedTile == tileEntity ? glm::vec4{1.f, 1.f, 0.f, 1.f} : registry.has<Blocked>(tileEntity) ? glm::vec4{1.f, 0.f, 0.f, 1.f} : glm::vec4{0.f, 0.f, 0.f, 1.f});
                render(lineRenderData);
            }
        }

        glDisable(GL_DEPTH_TEST);
    }
    GameState& gameState;
    TextureCatalog& textureCatalog;
    unsigned int unlitColorShader;
    unsigned int unlitTextureShader;
    unsigned int texBuffer;
    Entity selectedTile = 0;
    glm::ivec2 selectedPosition {-1, -1};
    TileType selectedTileType;
    RenderData lineRenderData;
    RenderData tileRenderData;
    Entity tink, george;
};

struct DialogSystem
{
    void run(Registry& registry, float deltaTime)
    {
        glUseProgram(unlitTextureShader);
        auto projection = glm::ortho(0.0f, 1920.0f, 1080.0f, 0.0f);
        auto position = Pos{10.f, 1080.f - font.common.lineHeight - 10};
        projection = glm::translate(projection, glm::vec3(position.x, position.y, 0.f));
        projection = glm::scale(projection, glm::vec3(0.5f, 0.5f, 1.f));
        setUniform(unlitTextureShader, "projection", projection);
        auto comicSansTexture = getTexture(fontTextureCatalog, "ComicSans80/ComicSans80_0.png");
        renderText(dialog, font, charTexBuffer, unlitTextureShader, comicSansTexture, 1.f / 1080.f, font.common.lineHeight, charRenderData);
    }
    BMFont& font;
    TextureCatalog& fontTextureCatalog;
    unsigned int unlitTextureShader;
    unsigned int charTexBuffer;
    unsigned int comicSansTexture;
    RenderData charRenderData;
    std::string dialog = "";
};

struct MissionSystem
{
    void run(Registry &registry, float deltaTime)
    {
        auto& tinkPos = registry.get<Pos>(tink);
        auto& georgePos = registry.get<Pos>(george);
        auto& ovenPos = registry.get<glm::ivec2>(oven);
        bool closeToGeorge = glm::length(tinkPos - georgePos) < 4.f;
        bool closeToOven = glm::length(tinkPos - glm::vec2{ovenPos.x, ovenPos.y}) < 5.f;
        gameState.allowMovement = true;
        switch (gameState.mission)
        {
            case Missions::START:
                dialogSystem.dialog = "Hey, is that George over there?\nLet's have a chat with him!";
                if (closeToGeorge)
                    gameState.mission = Missions::CHAT_GEORGE_1;
                break;
            case Missions::CHAT_GEORGE_1:
                dialogSystem.dialog = "Hey Tink,\nHave you heard about the lesser pottery throw down!!?";
                gameState.allowMovement = false;
                if (isPressed(GLFW_KEY_ENTER))
                {
                    gameState.mission = Missions::CHAT_GEORGE_2;
                }
                break;
            case Missions::CHAT_GEORGE_2:
                dialogSystem.dialog = "You should make a piece as well!!\nThe jury will love your style!";
                gameState.allowMovement = false;
                if (isPressed(GLFW_KEY_ENTER))
                {
                    gameState.mission = Missions::CHAT_GEORGE_3;
                }
                break;
            case Missions::CHAT_GEORGE_3:
                dialogSystem.dialog = "Start by gathering some wood for the oven!\nIf you have 5 big pieces, visit me for the next steps!";
                gameState.allowMovement = false;
                if (isPressed(GLFW_KEY_ENTER))
                {
                    gameState.mission = Missions::GATHER_WOOD;
                }
                break;
            case Missions::GATHER_WOOD:
            {
                std::stringstream ss;
                ss << "\nWood gathered: " << gameState.woodGathered << "/5" << std::endl;
                dialogSystem.dialog = ss.str();
                if (gameState.woodGathered >= 5 && closeToGeorge)
                {
                    gameState.woodGathered = 0;
                    gameState.mission = Missions::CHAT_GEORGE_4;
                }
                break;
            }
            case Missions::CHAT_GEORGE_4:
                dialogSystem.dialog = "Great! You have the wood!\nAs you know, we need clay to make a piece...";
                gameState.allowMovement = false;
                if (isPressed(GLFW_KEY_ENTER))
                {
                    gameState.mission = Missions::CHAT_GEORGE_5;
                }
                break;
            case Missions::CHAT_GEORGE_5:
                dialogSystem.dialog = "I guess you can find some clay near the riverbanks!\nGo checkout the banks on the other side of the river!";
                gameState.allowMovement = false;
                if (isPressed(GLFW_KEY_ENTER))
                {
                    gameState.mission = Missions::GATHER_CLAY;
                }
                break;
            case Missions::GATHER_CLAY:
            {
                std::stringstream ss;
                ss << "\nClay gathered: " << gameState.clayGathered << "/5" << std::endl;
                dialogSystem.dialog = ss.str();
                if (gameState.clayGathered >= 5 && closeToGeorge)
                {
                    gameState.clayGathered = 0;
                    gameState.mission = Missions::CHAT_GEORGE_6;
                }
                break;
            }
            case Missions::CHAT_GEORGE_6:
                dialogSystem.dialog = "How nice! Now you are ready to make your piece!\nWhen ready, go to the oven for the bisque firing! ";
                gameState.allowMovement = false;
                if (isPressed(GLFW_KEY_ENTER))
                {
                    gameState.mission = Missions::BAKE_PIECE;
                }
                break;
            case Missions::BAKE_PIECE:
                dialogSystem.dialog = "\nGo to the oven for the bisque firing";
                if (closeToOven)
                {
                    gameState.mission = Missions::BAKE_PIECE_HAND_IN;
                }
                break;
            case Missions::BAKE_PIECE_HAND_IN:
                dialogSystem.dialog = "\nShow your piece to George";
                if (closeToGeorge)
                {
                    gameState.mission = Missions::CHAT_GEORGE_7;
                }
                break;
            case Missions::CHAT_GEORGE_7:
                dialogSystem.dialog = "Wow, look at that!\nYou are ready for the glazing!";
                gameState.allowMovement = false;
                if (isPressed(GLFW_KEY_ENTER))
                {
                    gameState.mission = Missions::CHAT_GEORGE_8;
                }
                break;
            case Missions::CHAT_GEORGE_8:
                dialogSystem.dialog = "Now go get some wood and glaze materials.\nYou can find the material on the island up north.";
                gameState.allowMovement = false;
                if (isPressed(GLFW_KEY_ENTER))
                {
                    gameState.mission = Missions::GATHER_WOOD_AND_GLAZE;
                }
                break;
            case Missions::GATHER_WOOD_AND_GLAZE:
            {
                std::stringstream ss;
                ss << "Wood gathered: " << gameState.woodGathered << "/5\nGlaze materials gathered: " << gameState.glazeGathered << "/5" << std::endl;
                dialogSystem.dialog = ss.str();
                if (gameState.woodGathered >= 5 && gameState.glazeGathered >= 5 && closeToGeorge)
                {
                    gameState.clayGathered = 0;
                    gameState.mission = Missions::CHAT_GEORGE_9;
                }
                break;
            }
            case Missions::CHAT_GEORGE_9:
                dialogSystem.dialog = "Alright, let me make a nice glaze out of these materials.\nHere you go, now apply the glaze and get ready for the final firing!";
                gameState.allowMovement = false;
                if (isPressed(GLFW_KEY_ENTER))
                {
                    gameState.mission = Missions::GLAZE_PIECE;
                }
                break;
            case Missions::GLAZE_PIECE:
                dialogSystem.dialog = "\nGo to the oven for the glaze firing";
                if (closeToOven)
                {
                    gameState.mission = Missions::GLAZE_PIECE_HAND_IN;
                }
                break;
            case Missions::GLAZE_PIECE_HAND_IN:
                dialogSystem.dialog = "\nShow your piece to George";
                if (closeToGeorge)
                {
                    gameState.mission = Missions::SHOW_GEORGE;
                }
                break;
            case Missions::SHOW_GEORGE:
                dialogSystem.dialog = "Wow, look at that!\nThat looks amazing!!";
                gameState.allowMovement = false;
                if (isPressed(GLFW_KEY_ENTER))
                {
                    gameState.mission = Missions::CHAT_GEORGE_JURY;
                }
                break;
            case Missions::CHAT_GEORGE_JURY:
                dialogSystem.dialog = "...The jury?? The two of us already look the same.\nAdding a third character with the same sprite is weird...";
                gameState.allowMovement = false;
                if (isPressed(GLFW_KEY_ENTER))
                {
                    gameState.mission = Missions::CHAT_GEORGE_YOU_WON;
                }
                break;
            case Missions::CHAT_GEORGE_YOU_WON:
                dialogSystem.dialog = "I AM THE JURY-MENEER!!\nAnd you won the lesser pottery throw down!! Congratz!";
                gameState.allowMovement = false;
                if (isPressed(GLFW_KEY_ENTER))
                {
                    gameState.mission = Missions::END;
                }
                break;
            case Missions::END:
                dialogSystem.dialog = "\nPress F2 if you want to play again.";
                gameState.allowMovement = false;
                break;
            default:
                dialogSystem.dialog = "";
                break;
        }
    }
    GameState& gameState;
    DialogSystem& dialogSystem;
    Entity tink;
    Entity george;
    Entity oven;
};

struct WoodGatheringSystem
{
    void run(Registry &registry, float deltaTime)
    {
        if ((gameState.mission != Missions::GATHER_WOOD && gameState.mission != Missions::GATHER_WOOD_AND_GLAZE) || gameState.woodGathered >= 5)
            return;
        auto& tinkPos = registry.get<Pos>(tink);
        auto tiles = registry.each<DecoType>();
        for (auto [woodEntity, _]: tiles)
        {
            auto type = registry.get<DecoType>(woodEntity);
            auto pos = registry.get<glm::ivec2>(woodEntity);
            if (glm::length(tinkPos - glm::vec2{pos.x, pos.y}) < 1.1f && type == DecoType::WOOD)
            {
                registry.remove(woodEntity);
                gameState.woodGathered++;
            }
        }
    }
    Entity tink;
    GameState& gameState;
};

struct ClayGatheringSystem
{
    void run(Registry &registry, float deltaTime)
    {
        if (gameState.mission != Missions::GATHER_CLAY || gameState.clayGathered >= 5)
            return;
        auto& tinkPos = registry.get<Pos>(tink);
        auto tiles = registry.each<TileType>();
        for (auto [clayEntity, _]: tiles)
        {
            auto type = registry.get<TileType>(clayEntity);
            auto pos = registry.get<glm::ivec2>(clayEntity);
            if (glm::length(tinkPos - glm::vec2{pos.x, pos.y}) < 1.1f && type == TileType::CLAY)
            {
                registry.replace<TileType>(clayEntity, TileType::PATH);
                gameState.clayGathered++;
            }
        }
    }
    Entity tink;
    GameState& gameState;
};

struct GlazeGatheringSystem
{
    void run(Registry &registry, float deltaTime)
    {
        if (gameState.mission != Missions::GATHER_WOOD_AND_GLAZE || gameState.glazeGathered >= 5)
            return;
        auto& tinkPos = registry.get<Pos>(tink);
        auto tiles = registry.each<DecoType>();
        for (auto [glazeEntity, _]: tiles)
        {
            auto type = registry.get<DecoType>(glazeEntity);
            auto pos = registry.get<glm::ivec2>(glazeEntity);
            if (glm::length(tinkPos - glm::vec2{pos.x, pos.y}) < 1.1f && type == DecoType::GLAZE)
            {
                registry.remove(glazeEntity);
                gameState.glazeGathered++;
            }
        }
    }
    Entity tink;
    GameState& gameState;
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
            auto projection = glm::ortho(0.f, 30*1920.f/1080.f, 30.f, 0.f);
            setUniform(shaderID, "projection", projection);
            setUniform(shaderID, "color", color);
            render(renderData);
        }
    }
    unsigned int shaderID;
};
#endif
