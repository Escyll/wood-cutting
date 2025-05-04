
// TODO: Move ECS and Renderer to library, keep Systems in app

#include <random>
#include <sstream>
#include <fstream>

#include "ECS/Systems/Systems.h"

#include "ECS/ECS.h"
#include "Catalog.h"
#include "Renderer/Renderer.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "ECS/Systems/InputSystem.h"

using Color = glm::vec4;
using Pos = glm::vec2;

bool editing = false;

void MovementSystem::run(Registry &registry, float deltaTime)
{
    if (editing || !gameState.allowMovement)
        return;
    auto& pos = registry.get<Pos>(tink);
    glm::vec2 displacement {0.f, 0.f};
    if (isHolded(GLFW_KEY_W))
    {
        displacement.y += 1;
    }
    if (isHolded(GLFW_KEY_S))
    {
        displacement.y -= 1;
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
        auto worldDisplacement = speed * deltaTime * glm::normalize(displacement);
        auto newPos = pos + worldDisplacement;
        glm::ivec2 tilePos { newPos.x, newPos.y };
        for (auto [_, pos, __] : registry.each<glm::ivec2, Blocked>())
        {
            if (pos == tilePos)
                return;
        }
        if (camera)
        {
            camera->position = newPos;
        }
        pos = newPos;
    }
}

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
        Layer layer;
        wf.read(reinterpret_cast<char*>(&pos), sizeof(pos));
        wf.read(reinterpret_cast<char*>(&type), sizeof(type));
        wf.read(reinterpret_cast<char*>(&layer), sizeof(layer));
        auto tile = registry.create();
        registry.insert<glm::ivec2>(tile, pos);
        registry.insert<TileType>(tile, type);
        registry.insert<Layer>(tile, layer);
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
        Layer layer;
        wf.read(reinterpret_cast<char*>(&pos), sizeof(pos));
        wf.read(reinterpret_cast<char*>(&type), sizeof(type));
        wf.read(reinterpret_cast<char*>(&layer), sizeof(layer));
        auto tile = registry.create();
        registry.insert<glm::ivec2>(tile, pos);
        registry.insert<DecoType>(tile, type);
        registry.insert<Layer>(tile, layer);
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

void TileEditingSystem::selectTile(const glm::ivec2& nextSelectedPosition, Registry &registry)
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

TileType TileEditingSystem::typeOfNeighbor(const glm::ivec2& neighbour, Registry& registry)
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

void TileEditingSystem::run(Registry &registry, float deltaTime)
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
            int layer = 0;
            wf.write(reinterpret_cast<char*>(&layer), sizeof(layer));
        }
        auto decoTiles = registry.each<glm::ivec2, DecoType>();
        count = decoTiles.size();
        wf.write(reinterpret_cast<const char*>(&count), sizeof(count));
        for (auto [tileEntity, pos, decoType]: decoTiles)
        {
            wf.write(reinterpret_cast<char*>(&pos), sizeof(pos));
            wf.write(reinterpret_cast<char*>(&decoType), sizeof(decoType));
            int layer = decoType == DecoType::OVEN ? 2 : 1;
            wf.write(reinterpret_cast<char*>(&layer), sizeof(layer));
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

    // Render grid
    Render::Material mat;
    mat.name = "Grid";
    mat.shader = unlitColorShader;
    mat.renderData = lineRenderData;

    Render::setCamera(camera);
    Render::setLayer(1);

    for (auto [tileEntity, pos, type]: registry.each<glm::ivec2, TileType>())
    {
        Render::setSubLayer(0);
        mat.uniform4fs["color"] = glm::vec4{0.f, 0.f, 0.f, 1.f};
        if (registry.has<Blocked>(tileEntity))
        {
            Render::setSubLayer(1);
            mat.uniform4fs["color"] = glm::vec4{1.f, 0.f, 0.f, 1.f};
        }
        if (selectedTile == tileEntity)
        {
            Render::setSubLayer(2);
            mat.uniform4fs["color"] = glm::vec4{1.f, 1.f, 0.f, 1.f};
        }

        if (editing)
        {
            if (selectedTile == 0)
            {
                selectedTile = tileEntity;
                selectedPosition = pos;
            }
            Render::setMaterial(mat);
            std::vector<glm::vec2> positions = {glm::vec2{pos}, glm::vec2{pos} + glm::vec2{1.0, 0.0}, glm::vec2{pos} + glm::vec2{1.0, 1.0}, glm::vec2{pos} + glm::vec2{0.0, 1.0}};
            Render::queue({positions[0], positions[1], positions[1], positions[2], positions[2], positions[3], positions[3], positions[0]});
        }
    }
    Render::flush();
}

std::map<TileType, AtlasInfo> tileAtlasInfoMap
{
    {TileType::GRASS, { "Cute_Fantasy_Free/Tiles/Grass_Middle.png", {0, 0}, {1, 1}, {1, 1} } },
    {TileType::WATER, { "Cute_Fantasy_Free/Tiles/Water_Middle.png", {0, 0}, {1, 1}, {1, 1} } },
    {TileType::PATH, { "Cute_Fantasy_Free/Tiles/Path_Middle.png", {0, 0}, {1, 1}, {1, 1} } },
    {TileType::CLAY, { "Cute_Fantasy_Free/Tiles/Path_Tile.png", {2, 5}, {1, 1}, {3, 6} } },
    {TileType::GRASS_WATER_NW, { "Cute_Fantasy_Free/Tiles/Water_Tile.png", {0, 3}, {1, 1}, {3, 6} } },
    {TileType::GRASS_WATER_N, { "Cute_Fantasy_Free/Tiles/Water_Tile.png", {1, 2}, {1, 1}, {3, 6} } },
    {TileType::GRASS_WATER_NE, { "Cute_Fantasy_Free/Tiles/Water_Tile.png", {1, 3}, {1, 1}, {3, 6} } },
    {TileType::GRASS_WATER_E, { "Cute_Fantasy_Free/Tiles/Water_Tile.png", {0, 1}, {1, 1}, {3, 6} } },
    {TileType::WATER_GRASS_SE, { "Cute_Fantasy_Free/Tiles/Water_Tile.png", {2, 2}, {1, 1}, {3, 6} } },
    {TileType::WATER_GRASS_SW, { "Cute_Fantasy_Free/Tiles/Water_Tile.png", {0, 2}, {1, 1}, {3, 6} } },
    {TileType::PATH_WATER_W, { "Cute_Fantasy_Free/Tiles/Beach_Tile.png", {0, 1}, {1, 1}, {5, 3} } },
    {TileType::PATH_WATER_E, { "Cute_Fantasy_Free/Tiles/Beach_Tile.png", {2, 1}, {1, 1}, {5, 3} } },
    {TileType::PATH_WATER_SE, { "Cute_Fantasy_Free/Tiles/Beach_Tile.png", {2, 2}, {1, 1}, {5, 3} } },
    {TileType::PATH_WATER_S, { "Cute_Fantasy_Free/Tiles/Beach_Tile.png", {1, 2}, {1, 1}, {5, 3} } },
    {TileType::PATH_WATER_SW, { "Cute_Fantasy_Free/Tiles/Beach_Tile.png", {0, 2}, {1, 1}, {5, 3} } },
    {TileType::WATER_PATH_NE, { "Cute_Fantasy_Free/Tiles/Beach_Tile.png", {4, 0}, {1, 1}, {5, 3} } },
    {TileType::WATER_PATH_NW, { "Cute_Fantasy_Free/Tiles/Beach_Tile.png", {3, 0}, {1, 1}, {5, 3} } },
    {TileType::PATH_GRASS_NE, { "Cute_Fantasy_Free/Tiles/Path_Tile.png", {2, 0}, {1, 1}, {3, 6} } },
    {TileType::GRASS_PATH_W, { "Cute_Fantasy_Free/Tiles/Path_Tile.png", {2, 1}, {1, 1}, {3, 6} } },
    {TileType::GRASS_PATH_SW, { "Cute_Fantasy_Free/Tiles/Path_Tile.png", {0, 4}, {1, 1}, {3, 6} } },
    {TileType::GRASS_PATH_S, { "Cute_Fantasy_Free/Tiles/Path_Tile.png", {1, 0}, {1, 1}, {3, 6} } }
};

std::map<DecoType, AtlasInfo> decoAtlasInfoMap
{
    {DecoType::WOOD, { "Cute_Fantasy_Free/Outdoor decoration/Outdoor_Decor_Free.png", {0, 7}, {2, 1}, {7, 12}, {2, 1} } },
    {DecoType::GLAZE, { "Cute_Fantasy_Free/Outdoor decoration/Outdoor_Decor_Free.png", {0, 4}, {1, 1}, {7, 12} } },
    {DecoType::FLOWER, { "Cute_Fantasy_Free/Outdoor decoration/Outdoor_Decor_Free.png", {0, 10}, {1, 1}, {7, 12} } },
    {DecoType::OVEN, { "Oven.png", {0, 0}, {1, 1}, {1, 1}, {3, 3} } },
    {DecoType::BRIDGE_HOR, { "Cute_Fantasy_Free/Outdoor decoration/Bridge_Wood.png", {0, 1}, {3, 3}, {9, 4}, {5, 5}, { -.5f, -1.f } } },
    {DecoType::BRIDGE_VER, { "Cute_Fantasy_Free/Outdoor decoration/Bridge_Wood.png", {3, 1}, {3, 3}, {9, 4}, {5, 5}, { -1.f, -0.5f } } }
};

std::vector<glm::vec2> TileSystem::toTextureCoord(const glm::ivec2& tilePos, const glm::ivec2 tileCount, const glm::ivec2& span)
{
    return {
        glm::vec2{(float) tilePos.x / tileCount.x, (float) tilePos.y / tileCount.y},
        glm::vec2{((float) tilePos.x + span.x) / tileCount.x, (float) tilePos.y / tileCount.y},
        glm::vec2{((float) tilePos.x + span.x) / tileCount.x, ((float) tilePos.y + span.y) / tileCount.y},
        glm::vec2{((float) tilePos.x + span.x) / tileCount.x, ((float) tilePos.y + span.y) / tileCount.y},
        glm::vec2{(float) tilePos.x / tileCount.x, ((float) tilePos.y + span.y) / tileCount.y},
        glm::vec2{(float) tilePos.x / tileCount.x, (float) tilePos.y / tileCount.y}
    };
}

std::vector<glm::vec2> TileSystem::toPosCoord(const glm::vec2& pos)
{
    return {
        glm::vec2{pos.x, pos.y},
        glm::vec2{pos.x + 1, pos.y},
        glm::vec2{pos.x + 1, pos.y + 1},
        glm::vec2{pos.x + 1, pos.y + 1},
        glm::vec2{pos.x, pos.y + 1},
        glm::vec2{pos.x, pos.y},
    };
}

std::vector<glm::vec2> TileSystem::toPosCoord(const glm::vec2& pos, const glm::vec2& size, const glm::vec2& translation)
{
    return {
        glm::vec2{pos.x + translation.x, pos.y + translation.y},
        glm::vec2{pos.x + translation.x + size.x, pos.y + translation.y},
        glm::vec2{pos.x + translation.x + size.x, pos.y + translation.y + size.y},
        glm::vec2{pos.x + translation.x + size.x, pos.y + translation.y + size.y},
        glm::vec2{pos.x + translation.x, pos.y + translation.y + size.y},
        glm::vec2{pos.x + translation.x, pos.y + translation.y},
    };
}

void TileSystem::run(Registry &registry, float deltaTime)
{
    std::unordered_map<unsigned int, std::vector<glm::vec3>> posCoords;
    std::unordered_map<unsigned int, std::vector<glm::vec2>> texCoords;
    // Render tiles

    Render::Material mat;
    mat.name = "Base";
    mat.shader = unlitTextureShader;
    mat.renderData = tileRenderData;

    Render::setCamera(camera);

    for (auto [tileEntity, pos, type, layer]: registry.each<glm::ivec2, TileType, Layer>())
    {
        Render::setLayer(layer.layer);
        Render::setSubLayer(layer.layer == 2 ? pos.y : 0);
        auto& atlasInfo = tileAtlasInfoMap[type];
        unsigned int texture = getTexture(textureCatalog, atlasInfo.texture);
        mat.name = atlasInfo.texture;
        mat.texture = texture;
        Render::setMaterial(mat);
        Render::queue(toPosCoord(pos), toTextureCoord(atlasInfo.pos, atlasInfo.atlasSize));
    }

    for (auto [tileEntity, pos, type, layer]: registry.each<glm::ivec2, DecoType, Layer>())
    {
        auto& atlasInfo = decoAtlasInfoMap[type];
        unsigned int texture = getTexture(textureCatalog, atlasInfo.texture);
        mat.name = atlasInfo.texture;
        mat.texture = texture;
        Render::setLayer(layer.layer);
        Render::setSubLayer(layer.layer == 2 ? pos.y + atlasInfo.spriteSize.y + atlasInfo.spriteTranslate.y : 0); // TODO Check this
        Render::setMaterial(mat);
        auto posCoords = toPosCoord(pos, atlasInfo.spriteSize, atlasInfo.spriteTranslate);
        auto texCoords = toTextureCoord(atlasInfo.pos, atlasInfo.atlasSize, atlasInfo.span);
        Render::queue(posCoords, texCoords);
    }     

    {
        unsigned int texture = getTexture(textureCatalog, "Cute_Fantasy_Free/Player/Player.png");
        mat.name = "Cute_Fantasy_Free/Player/Player.png";
        mat.texture = texture;
        Render::setLayer(2);
        
        auto pos = registry.get<Pos>(george);
        auto texCoords = toTextureCoord({1, 8}, {6, 10}, {1, 1});
        auto posCoords = toPosCoord(pos, {3, 3}, {-1.5f, -2.f});
        Render::setSubLayer(pos.y + 3);
        Render::setMaterial(mat); // TODO Should this be reset by layer and sublayer?
        Render::queue(posCoords, texCoords);

        pos = registry.get<Pos>(tink);
        auto animation = registry.get<AnimationState>(tink);
        auto frame = animation.currentFrame;
        glm::vec2 bottomLeft = frame.textureRegion.bottomLeft;
        glm::vec2 bottomRight = { bottomLeft.x + frame.textureRegion.size.x, bottomLeft.y };
        glm::vec2 topLeft = { bottomLeft.x, bottomLeft.y + frame.textureRegion.size.y };
        glm::vec2 topRight = bottomLeft + frame.textureRegion.size;

        texCoords = { bottomLeft, bottomRight, topRight, topRight, topLeft, bottomLeft };
        posCoords = toPosCoord(pos, {3, 3}, {-1.5f, -2.f});
        Render::setSubLayer(pos.y + 3);
        Render::setMaterial(mat); // TODO Should this be reset by layer and sublayer?
        Render::queue(posCoords, texCoords);
    }
    Render::flush();
}

void DialogSystem::run(Registry& registry, float deltaTime)
{
    glUseProgram(unlitTextureShader);
    Render::setCamera(camera);
    auto comicSansTexture = getTexture(fontTextureCatalog, "ComicSans80/ComicSans80_0.png");
    renderText(dialog, font, unlitTextureShader, comicSansTexture, charRenderData);
}

void MissionSystem::run(Registry &registry, float deltaTime)
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

void AnimationSystem::run(Registry &registry, float deltaTime)
{
    for (auto [entity, animationState] : registry.each<AnimationState>())
    {
        auto& animationSequence = getAnimation(catalog, animationState.animation);
        animationState.elapsedTime += deltaTime;
        if (animationState.elapsedTime > animationSequence.duration)
        {
            animationState.elapsedTime -= animationSequence.duration;
        }
        auto accumulatedTime = 0.f;
        Frame currentFrame = animationSequence.frames[0];
        for (auto &frame : animationSequence.frames)
        {
            if (accumulatedTime < animationState.elapsedTime)
            {
                currentFrame = frame;
            }
            accumulatedTime += frame.duration;
        }
        animationState.currentFrame = currentFrame;
        registry.replace(entity, animationState);
    }
}

void WoodGatheringSystem::run(Registry &registry, float deltaTime)
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

void ClayGatheringSystem::run(Registry &registry, float deltaTime)
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

void GlazeGatheringSystem::run(Registry &registry, float deltaTime)
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
