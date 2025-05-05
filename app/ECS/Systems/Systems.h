// TODO: Move ECS and Renderer to library, keep Systems in app

#pragma once

#include "ECS/ECS.h"
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

struct Layer
{
    int layer;
};

struct Tree {};

struct Blocked{};

struct MovementSystem
{
    void run(Registry &registry, float deltaTime);
    GameState& gameState;
    float speed = 4.f;
    Entity tink;
    Render::Camera* camera = nullptr;
};

void loadLevel(Registry& registry);

struct TileEditingSystem
{
    void selectTile(const glm::ivec2& nextSelectedPosition, Registry &registry);
    TileType typeOfNeighbor(const glm::ivec2& neighbour, Registry& registry);
    void run(Registry &registry, float deltaTime);
    GameState& gameState;
    RenderData lineRenderData;
    unsigned int unlitColorShader;
    Entity selectedTile = 0;
    glm::ivec2 selectedPosition {-1, -1};
    TileType selectedTileType;
    Entity tink;
    Render::Camera* camera = nullptr;
};

struct AtlasInfo
{
    std::string texture;
    glm::ivec2 pos;
    glm::ivec2 span;
    glm::ivec2 atlasSize; // TODO JH: Maybe move to texture info instead of AtlasInfo. Now duplicated a lot.
    glm::vec2 spriteSize = {1, 1};
    glm::vec2 spriteTranslate = {0, 0};
};

struct TileSystem
{
    std::vector<glm::vec2> toTextureCoord(const glm::ivec2& tilePos, const glm::ivec2 tileCount, const glm::ivec2& span = {1, 1});
    std::vector<glm::vec2> toPosCoord(const glm::vec2& pos);
    std::vector<glm::vec2> toPosCoord(const glm::vec2& pos, const glm::vec2& size, const glm::vec2& translation);

    void run(Registry &registry, float deltaTime);
    GameState& gameState;
    TextureCatalog& textureCatalog;
    unsigned int unlitTextureShader;
    unsigned int posBuffer;
    unsigned int texBuffer;
    RenderData tileRenderData;
    Entity tink, george;
    Render::Camera* camera = nullptr;
};

struct DialogSystem
{
    void run(Registry& registry, float deltaTime);
    BMFont& font;
    TextureCatalog& fontTextureCatalog;
    unsigned int unlitTextureShader;
    unsigned int charTexBuffer;
    unsigned int comicSansTexture;
    RenderData charRenderData;
    std::string dialog = "";
    Render::Camera* camera = nullptr;
};

struct MissionSystem
{
    void run(Registry &registry, float deltaTime);
    GameState& gameState;
    DialogSystem& dialogSystem;
    Entity tink;
    Entity george;
    Entity oven;
};

struct AnimationSystem
{
    void run(Registry &registry, float deltaTime);
    AnimationCatalog& catalog;
};

struct WoodGatheringSystem
{
    void run(Registry &registry, float deltaTime);
    Entity tink;
    GameState& gameState;
};

struct ClayGatheringSystem
{
    void run(Registry &registry, float deltaTime);
    Entity tink;
    GameState& gameState;
};

struct GlazeGatheringSystem
{
    void run(Registry &registry, float deltaTime);
    Entity tink;
    GameState& gameState;
};
