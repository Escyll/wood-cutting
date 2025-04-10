#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <cmath>

#include "Renderer/Renderer.h"
#include "ECS/ECS.h"
#include "ECS/Systems/Systems.h"
#include "ECS/Systems/InputSystem.h"
#include "Renderer/Shaders.h"
#include "Renderer/Textures.h"
#include "Renderer/Window.h"
#include "Geometry.h"
#include "Catalog.h"
#include "FontRendering/BMFont.h"

int main(int argc, char *argv[])
{
    auto window = initializeOpenGLAndCreateWindow();

    if (not window)
        return -1;

    glfwSetKeyCallback(window, keyCallback);

    auto textureCatalog = createTextureCatalog("assets/textures");
    auto animationCatalog = createAnimationCatalog("assets/textures");
    auto fontTextureCatalog = createTextureCatalog("assets/fonts");
    auto font = loadBMFont("assets/fonts/ComicSans80/ComicSans80.fnt");

    auto unlitColorVertex = readFile("assets/shaders/unlit-color/vertex.glsl");
    auto unlitColorFragment = readFile("assets/shaders/unlit-color/fragment.glsl");
    auto unlitTextureVertex = readFile("assets/shaders/unlit-texture/vertex.glsl");
    auto unlitTextureFragment = readFile("assets/shaders/unlit-texture/fragment.glsl");

    auto unlitColorShader = createShaderProgram(unlitColorVertex.c_str(), unlitColorFragment.c_str());
    auto unlitTextureShader = createShaderProgram(unlitTextureVertex.c_str(), unlitTextureFragment.c_str());

    BufferData tileBuffer = bufferData(createRectangleVertices(1.f, 1.f));
    BufferData tileTexBuffer = bufferData({{0.0, 0.0}, {1.0, 0.0}, {1.0, 1.0}, {0.1, 1.0}});
    BufferData rectangleIndexBuffer = bufferIndexData({0,1,2,2,3,0});
    BufferData rectangleLineIndexBuffer = bufferIndexData({0,1,1,2,2,3,3,0});
    BufferData charPosBuffer = bufferData(createRectangleVertices(font.common.lineHeight, font.common.lineHeight));
    BufferData charTexBuffer = bufferData({{0.2, 0.2}, {0.2, 0.5}, {0.5, 0.5}, {0.5, 0.2}});

    auto tileLineVAO = createPosVAO(tileBuffer.handle, rectangleLineIndexBuffer.handle);
    auto tileVAO = createPosTexVAO(tileBuffer.handle, tileTexBuffer.handle);
    auto charVAO = createPosTexVAO(charPosBuffer.handle, charTexBuffer.handle, rectangleIndexBuffer.handle);
    RenderData charRenderData { charVAO, charPosBuffer.handle, charTexBuffer.handle, GL_TRIANGLES };
    RenderData tileRenderData { tileVAO, tileBuffer.handle, tileTexBuffer.handle, GL_TRIANGLES };
    RenderData tileLineRenderData { tileLineVAO, tileBuffer.handle, 0, GL_LINES };

    GameState gameState;
    gameState.mission = Missions::START;

    TileSystem tileSystem { gameState, textureCatalog };
    tileSystem.unlitTextureShader = unlitTextureShader;
    tileSystem.posBuffer = tileBuffer.handle;
    tileSystem.texBuffer = tileTexBuffer.handle;
    tileSystem.tileRenderData = tileRenderData;

    TileEditingSystem tileEditingSystem { gameState };
    tileEditingSystem.unlitColorShader = unlitColorShader;
    tileEditingSystem.lineRenderData = tileLineRenderData;

    Registry registry;
    Entity tink = registry.create();
    Entity george = registry.create();
    Entity oven = registry.create();
    
    tileSystem.tink = tink;
    tileSystem.george = george;

    registry.insert<Pos>(tink, Pos{25, 20});
    registry.insert<Pos>(george, Pos{18, 7});
    registry.insert<glm::ivec2>(oven, {30, 30});
    registry.insert<ActiveAnimation>(tink, { "Cute_Fantasy_Free/Player/RunDown", 0 });

    MovementSystem movementSystem { gameState };
    movementSystem.tink = tink;
    WoodGatheringSystem woodGatheringSystem{tink, gameState};
    ClayGatheringSystem clayGatheringSystem{tink, gameState};
    GlazeGatheringSystem glazeGatheringSystem{tink, gameState};
    DialogSystem dialogSystem { font, fontTextureCatalog };
    dialogSystem.unlitTextureShader = unlitTextureShader;
    dialogSystem.charTexBuffer = charTexBuffer.handle;
    dialogSystem.charRenderData = charRenderData;
    MissionSystem missionSystem { gameState, dialogSystem };
    missionSystem.tink = tink;
    missionSystem.george = george;
    missionSystem.oven = oven;

    AnimationSystem animationSystem { animationCatalog };
    
    loadLevel(registry);

    glfwSwapInterval(0);
    auto previousFrame = 0.f;
    while (!glfwWindowShouldClose(window))
    {
        markKeyStatesHold();
        glfwPollEvents();
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        auto currentFrame = glfwGetTime();
        auto timeDelta = currentFrame - previousFrame;
        previousFrame = currentFrame;

        processInput(window);
        movementSystem.run(registry, timeDelta);
        woodGatheringSystem.run(registry, timeDelta);
        clayGatheringSystem.run(registry, timeDelta);
        glazeGatheringSystem.run(registry, timeDelta);
        missionSystem.run(registry, timeDelta);
        animationSystem.run(registry, timeDelta);

        glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        tileSystem.run(registry, timeDelta);
        tileEditingSystem.run(registry, timeDelta);
        dialogSystem.run(registry, timeDelta);

        glfwSwapBuffers(window);
    }
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
