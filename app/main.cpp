#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <cmath>

#include "ECS/ECS.h"
#include "ECS/Systems/Systems.h"
#include "Renderer/Renderer.h"
#include "Renderer/Shaders.h"
#include "Renderer/Textures.h"
#include "Geometry.h"
#include "FontRendering/BMFont.h"

int main(int argc, char *argv[])
{
    auto window = initializeOpenGLAndCreateWindow();
    if (not window)
        return -1;

    auto font = loadBMFont("assets/fonts/ComicSans80/ComicSans80.fnt");
    auto comicSansTexture = loadTexture("assets/fonts/ComicSans80/ComicSans80_0.png");
    auto grassTexture = loadTexture("assets/textures/Cute_Fantasy_Free/Tiles/Grass_Middle.png");
    auto waterTexture = loadTexture("assets/textures/Cute_Fantasy_Free/Tiles/Water_Middle.png");
    auto waterTileTexture = loadTexture("assets/textures/Cute_Fantasy_Free/Tiles/Water_Tile.png");
    auto pathTexture = loadTexture("assets/textures/Cute_Fantasy_Free/Tiles/Path_Middle.png");
    auto pathTileTexture = loadTexture("assets/textures/Cute_Fantasy_Free/Tiles/Path_Tile.png");
    auto beachTileTexture = loadTexture("assets/textures/Cute_Fantasy_Free/Tiles/Beach_Tile.png");
    auto outdoorDecorTexture = loadTexture("assets/textures/Cute_Fantasy_Free/Outdoor decoration/Outdoor_Decor_Free.png");

    auto unlitColorVertex = readFile("assets/shaders/unlit-color/vertex.glsl");
    auto unlitColorFragment = readFile("assets/shaders/unlit-color/fragment.glsl");
    auto unlitTextureVertex = readFile("assets/shaders/unlit-texture/vertex.glsl");
    auto unlitTextureFragment = readFile("assets/shaders/unlit-texture/fragment.glsl");

    auto unlitColorShader = createShaderProgram(unlitColorVertex.c_str(), unlitColorFragment.c_str());
    auto unlitTextureShader = createShaderProgram(unlitTextureVertex.c_str(), unlitTextureFragment.c_str());

    BufferData circleBuffer = bufferData(createCircleVertices(0.5f, 50));
    BufferData tinkBuffer = bufferData(createCircleVertices(0.5f, 50));
    BufferData tileBuffer = bufferData(createRectangleVertices(1.f, 1.f));
    BufferData tileTexBuffer = bufferData({{0.0, 0.0}, {1.0, 0.0}, {1.0, 1.0}, {0.1, 1.0}});
    BufferData rectangleIndexBuffer = bufferIndexData({0,1,2,2,3,0});
    BufferData rectangleLineIndexBuffer = bufferIndexData({0,1,1,2,2,3,3,0});
    BufferData charPosBuffer = bufferData(createRectangleVertices(font.common.lineHeight, font.common.lineHeight));
    BufferData charTexBuffer = bufferData({{0.2, 0.2}, {0.2, 0.5}, {0.5, 0.5}, {0.5, 0.2}});

    auto tinkVAO = createPosVAO(tinkBuffer.handle);
    auto tileLineVAO = createPosVAO(tileBuffer.handle, rectangleLineIndexBuffer.handle);
    auto tileVAO = createPosTexVAO(tileBuffer.handle, tileTexBuffer.handle, rectangleIndexBuffer.handle);
    auto charVAO = createPosTexVAO(charPosBuffer.handle, charTexBuffer.handle, rectangleIndexBuffer.handle);
    RenderData charRenderData { charVAO, rectangleIndexBuffer.elementCount, GL_TRIANGLES, DrawStrategy::ELEMENTS };
    RenderData tileRenderData { tileVAO, rectangleIndexBuffer.elementCount, GL_TRIANGLES, DrawStrategy::ELEMENTS };
    RenderData tileLineRenderData { tileLineVAO, rectangleLineIndexBuffer.elementCount, GL_LINES, DrawStrategy::ELEMENTS };

    RenderSystem renderSystem;
    renderSystem.shaderID = unlitColorShader;
    TileSystem tileSystem;
    tileSystem.unlitColorShader = unlitColorShader;
    tileSystem.unlitTextureShader = unlitTextureShader;
    tileSystem.texBuffer = tileTexBuffer.handle;
    tileSystem.grassTexture = grassTexture;
    tileSystem.waterTexture = waterTexture;
    tileSystem.waterTileTexture = waterTileTexture;
    tileSystem.pathTexture = pathTexture;
    tileSystem.pathTileTexture = pathTileTexture;
    tileSystem.beachTileTexture = beachTileTexture;
    tileSystem.window = window;
    tileSystem.lineRenderData = tileLineRenderData;
    tileSystem.tileRenderData = tileRenderData;
    tileSystem.outdoorDecorTexture = outdoorDecorTexture;

    Registry registry;
    Entity tink = registry.create();
    Entity george = registry.create();

    registry.insert<Pos>(tink, Pos{25, 20});
    registry.insert<Color>(tink, Color{1.0, 0.2, 0.3, 1.0});
    registry.insert<RenderData>(tink, {tinkVAO, circleBuffer.elementCount, GL_TRIANGLE_FAN});

    registry.insert<Pos>(george, Pos{18, 7});
    registry.insert<Color>(george, Color{0.2, 0.2, 1.0, 1.0});
    registry.insert<RenderData>(george, {tinkVAO, circleBuffer.elementCount, GL_TRIANGLE_FAN});

    GameState gameState;
    gameState.mission = Missions::START;

    MovementSystem movementSystem { gameState };
    movementSystem.tink = tink;
    movementSystem.window = window;
    WoodGatheringSystem woodGatheringSystem{tink, gameState};
    ClayGatheringSystem clayGatheringSystem{tink, gameState};
    GlazeGatheringSystem glazeGatheringSystem{tink, gameState};
    DialogSystem dialogSystem { font };
    dialogSystem.unlitTextureShader = unlitTextureShader;
    dialogSystem.charTexBuffer = charTexBuffer.handle;
    dialogSystem.comicSansTexture = comicSansTexture;
    dialogSystem.charRenderData = charRenderData;
    MissionSystem missionSystem { gameState, dialogSystem };
    missionSystem.tink = tink;
    missionSystem.george = george;
    missionSystem.window = window;

    glfwSwapInterval(1);
    auto previousFrame = 0.f;
    while (!glfwWindowShouldClose(window))
    {
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glfwPollEvents();
        auto currentFrame = glfwGetTime();
        auto timeDelta = currentFrame - previousFrame;
        previousFrame = currentFrame;

        processInput(window);
        movementSystem.run(registry, timeDelta);
        woodGatheringSystem.run(registry, timeDelta);
        clayGatheringSystem.run(registry, timeDelta);
        glazeGatheringSystem.run(registry, timeDelta);
        missionSystem.run(registry, timeDelta);

        glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        tileSystem.run(registry, timeDelta);
        renderSystem.run(registry, timeDelta);
        dialogSystem.run(registry, timeDelta);

        glfwSwapBuffers(window);
    }
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
