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

enum class Tile
{
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

int main(int argc, char *argv[])
{
    auto window = initializeOpenGLAndCreateWindow();
    if (not window)
        return -1;

    auto font = loadBMFont("assets/fonts/ComicSans80/ComicSans80.fnt");
    auto texture = loadTexture("assets/fonts/ComicSans80/ComicSans80_0.png");

    auto unlitColorVertex = readFile("assets/shaders/unlit-color/vertex.glsl");
    auto unlitColorFragment = readFile("assets/shaders/unlit-color/fragment.glsl");
    auto unlitTextureVertex = readFile("assets/shaders/unlit-texture/vertex.glsl");
    auto unlitTextureFragment = readFile("assets/shaders/unlit-texture/fragment.glsl");

    auto unlitColorShader = createShaderProgram(unlitColorVertex.c_str(), unlitColorFragment.c_str());
    auto unlitTextureShader = createShaderProgram(unlitTextureVertex.c_str(), unlitTextureFragment.c_str());

    BufferData circleBuffer = bufferData(createCircleVertices(50.f, 50));
    BufferData treeBuffer = bufferData(createCircleVertices(20.f, 50));
    BufferData tinkBuffer = bufferData(createCircleVertices(15.f, 50));
    BufferData lumberMillBuffer = bufferData(createRectangleVertices(1200.f, font.common.lineHeight + 10));
    BufferData rectangleIndexBuffer = bufferIndexData({0,1,2,2,3,0});
    BufferData charPosBuffer = bufferData(createRectangleVertices(font.common.lineHeight, font.common.lineHeight));
    BufferData charTexBuffer = bufferData({{0.2, 0.2}, {0.2, 0.5}, {0.5, 0.5}, {0.5, 0.2}});

    auto treeVAO = createPosVAO(treeBuffer.handle);
    auto tinkVAO = createPosVAO(tinkBuffer.handle);
    auto lumberMillVAO = createPosVAO(lumberMillBuffer.handle, rectangleIndexBuffer.handle);
    auto charVAO = createPosTexVAO(charPosBuffer.handle, charTexBuffer.handle, rectangleIndexBuffer.handle);
    RenderData charRenderData { charVAO, rectangleIndexBuffer.elementCount, GL_TRIANGLES, DrawStrategy::ELEMENTS };

    RenderSystem renderSystem;
    renderSystem.shaderID = unlitColorShader;
    //WoodCuttingSystem woodCuttingSystem;

    Registry registry;
    Entity lumberMill = registry.create();
    Entity tink = registry.create();

    registry.insert<Pos>(lumberMill, Pos{(1920.f - 1200.f) / 2.f, 1080.f - font.common.lineHeight - 10});
    registry.insert<Color>(lumberMill, Color{0.6, 0.2, 0.2, 1.0});
    registry.insert<RenderData>(lumberMill, {lumberMillVAO, rectangleIndexBuffer.elementCount, GL_TRIANGLES, DrawStrategy::ELEMENTS});

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<> disX(20.0, 400.0);
    std::uniform_real_distribution<> disY(20.0, 1060.0);
    for (int i = 0; i < 5; i++)
    {
        auto tree = registry.create();
        registry.insert<Tree>(tree, {});
        registry.insert<Pos>(tree, Pos{disX(gen), disY(gen)});
        registry.insert<Color>(tree, Color{0.1, 0.2, 0.1, 1.0});
        registry.insert<RenderData>(tree, {treeVAO, circleBuffer.elementCount, GL_TRIANGLE_FAN});
    }

    registry.insert<Pos>(tink, Pos{900, 500});
    registry.insert<Color>(tink, Color{1.0, 0.2, 0.3, 1.0});
    registry.insert<RenderData>(tink, {tinkVAO, circleBuffer.elementCount, GL_TRIANGLE_FAN});

    GameState gameState;

    MovementSystem movementSystem;
    movementSystem.tink = tink;
    movementSystem.window = window;
    WoodGatheringSystem woodGatheringSystem{tink, gameState};

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

        glClearColor(0.2f, 0.6f, 0.2f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        renderSystem.run(registry, timeDelta);

        glUseProgram(unlitTextureShader);
        auto projection = glm::ortho(0.0f, 1920.0f, 1080.0f, 0.0f);
        auto position = Pos{(1920.f - 1200.f) / 2.f, 1080.f - font.common.lineHeight - 10};
        projection = glm::translate(projection, glm::vec3(position.x, position.y, 0.f));
        projection = glm::scale(projection, glm::vec3(0.5f, 0.5f, 1.f));
        setUniform(unlitTextureShader, "projection", projection);
        renderText("Hey Tink,\nHave you heard about the lesser pottery throwdown!!?", font, charTexBuffer.handle, unlitTextureShader, texture, 1.f / 1080.f, font.common.lineHeight, charRenderData);

        glfwSwapBuffers(window);
    }
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
