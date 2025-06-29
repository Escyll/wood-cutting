#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <cmath>

#include "Platform.h"
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
#include "Imgui/Imgui.h"

void createFBO(unsigned int &fbo, unsigned int &fboBuffer) {
    glGenFramebuffers(1, &fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);

    glGenTextures(1, &fboTexture);
    glBindTexture(GL_TEXTURE_2D, fboTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, VIRTUAL_WIDTH, VIRTUAL_HEIGHT, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, fboTexture, 0);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        std::cerr << "Framebuffer not complete!\n";

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

int main(int argc, char *argv[])
{
    glfwWindowHint(GLFW_SAMPLES, 4);

    auto window = initializeOpenGLAndCreateWindow();

    if (not window)
        return -1;

    glfwSetKeyCallback(window, keyCallback);

    auto textureCatalog = createTextureCatalog("assets/textures", TEXTURE_FILTER::LINEAR);
    auto animationCatalog = createAnimationCatalog("assets/textures");
    auto fontTextureCatalog = createTextureCatalog("assets/fonts", TEXTURE_FILTER::LINEAR);
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
    RenderData shapeRenderData { tileLineVAO, tileBuffer.handle, 0, GL_TRIANGLES };

    Render::Camera sceneCamera { glm::vec3(0.f), Render::createProjection({640, 360}, 80, -100, 100), {640, 360} };
    auto downScaledFramebuffer = Render::createFramebuffer({640, 360});

    Render::Camera uiCamera { glm::vec3(0.f), glm::ortho(0.f, 1920.f, 1080.f, 0.f) };

    GameState gameState;
    gameState.mission = Missions::START;

    TileSystem tileSystem { gameState, textureCatalog };
    tileSystem.unlitTextureShader = unlitTextureShader;
    tileSystem.posBuffer = tileBuffer.handle;
    tileSystem.texBuffer = tileTexBuffer.handle;
    tileSystem.tileRenderData = tileRenderData;
    tileSystem.camera = &sceneCamera;

    TileEditingSystem tileEditingSystem { gameState };
    tileEditingSystem.unlitColorShader = unlitColorShader;
    tileEditingSystem.lineRenderData = tileLineRenderData;
    tileEditingSystem.camera = &sceneCamera;

    Registry registry;
    Entity tink = registry.create();
    Entity george = registry.create();
    Entity oven = registry.create();
    
    tileSystem.tink = tink;
    tileSystem.george = george;

    registry.insert<Pos>(tink, Pos{25, 20});
    registry.insert<Pos>(george, Pos{18, 7});
    registry.insert<glm::ivec2>(oven, {30, 30});
    registry.insert<AnimationState>(tink, { "Cute_Fantasy_Free/Player/RunDown", 0 });

    MovementSystem movementSystem { gameState };
    movementSystem.tink = tink;
    movementSystem.camera = &sceneCamera;
    WoodGatheringSystem woodGatheringSystem{tink, gameState};
    ClayGatheringSystem clayGatheringSystem{tink, gameState};
    GlazeGatheringSystem glazeGatheringSystem{tink, gameState};
    DialogSystem dialogSystem { font, fontTextureCatalog };
    dialogSystem.unlitTextureShader = unlitTextureShader;
    dialogSystem.charTexBuffer = charTexBuffer.handle;
    dialogSystem.charRenderData = charRenderData;
    dialogSystem.camera = &uiCamera;
    MissionSystem missionSystem { gameState, dialogSystem };
    missionSystem.tink = tink;
    missionSystem.george = george;
    missionSystem.oven = oven;

    AnimationSystem animationSystem { animationCatalog };
    
    loadLevel(registry);

    Imgui::installCallbacks(window);
    
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_MULTISAMPLE);

    glfwSwapInterval(0);
    auto previousFrame = 0.f;
    
    unsigned int fbo, fboTexture;
    createFBO(fbo, fboTexture);

    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();
        auto currentFrame = glfwGetTime();
        auto timeDelta = currentFrame - previousFrame;
        previousFrame = currentFrame;
        processInput(window);

        if (not windowSizeChangeHandled)
        {
            sceneCamera.projection = Render::createProjection({640, 360}, 40, -100, 100);
            uiCamera.projection = glm::ortho(0.f, (float) windowSize.x, (float) windowSize.y, 0.f);
            windowSizeChangeHandled = true;
        }

        // Game systems
        movementSystem.run(registry, timeDelta);
        woodGatheringSystem.run(registry, timeDelta);
        clayGatheringSystem.run(registry, timeDelta);
        glazeGatheringSystem.run(registry, timeDelta);
        missionSystem.run(registry, timeDelta);
        animationSystem.run(registry, timeDelta);

        // Render systems
        glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        tileSystem.run(registry, timeDelta);
        tileEditingSystem.run(registry, timeDelta);
        dialogSystem.run(registry, timeDelta);

        markKeyStatesHold();

        Imgui::begin(unlitColorShader, unlitTextureShader, tileRenderData, &uiCamera);
        Imgui::panelBegin("MyPanel", 10, 10, {Imgui::LayoutStyle::Column});

        if (Imgui::button("MyButton 1", 200, 100))
        {
            std::cerr << "Button press of MyButton 1 detected" << std::endl;
        }

        if (Imgui::button("MyButton 2", 150, 100))
        {
            std::cerr << "Button press of MyButton 2 detected" << std::endl;
        }

        int tileSize = 50;
        int id = 0;
        auto texture = getTexture(textureCatalog, "Cute_Fantasy_Free/Tiles/Path_Tile.png");

        Imgui::beginLayout({Imgui::LayoutStyle::Row});
        std::vector<Frame> frames = {
            getAnimation(animationCatalog, "Cute_Fantasy_Free/Tiles/grass_path_NW_SE").frames[0],
            getAnimation(animationCatalog, "Cute_Fantasy_Free/Tiles/grass_path_N_S").frames[0],
            getAnimation(animationCatalog, "Cute_Fantasy_Free/Tiles/grass_path_NE_SW").frames[0],
            getAnimation(animationCatalog, "Cute_Fantasy_Free/Tiles/path_dirt1").frames[0]
        };
        for (auto& frame : frames)
        {
            std::string name = "MyImageButton " + std::to_string(id++);
            if (Imgui::imageButton(name, texture, frame, tileSize, tileSize))
            {
                std::cerr << "Button press of " << name << " detected" << std::endl;
            }
        }
        Imgui::endLayout();

        Imgui::beginLayout({Imgui::LayoutStyle::Row});
        frames = {
            getAnimation(animationCatalog, "Cute_Fantasy_Free/Tiles/grass_path_SW_NE").frames[0],
            getAnimation(animationCatalog, "Cute_Fantasy_Free/Tiles/grass_path_S_N").frames[0],
            getAnimation(animationCatalog, "Cute_Fantasy_Free/Tiles/grass_path_SE_NW").frames[0]
        };
        for (auto& frame : frames)
        {
            std::string name = "MyImageButton " + std::to_string(id++);
            if (Imgui::imageButton(name, texture, frame, tileSize, tileSize))
            {
                std::cerr << "Button press of " << name << " detected" << std::endl;
            }
        }
        Imgui::endLayout();

        Imgui::panelEnd();

        Imgui::panelBegin("MyPanel 2", 1000, 10, {Imgui::LayoutStyle::Row});

        if (Imgui::button("MyButton 3", 200, 80))
        {
            std::cerr << "Button press of MyButton 3 detected" << std::endl;
        }

        if (Imgui::button("MyButton 4", 200, 100))
        {
            std::cerr << "Button press of MyButton 4 detected" << std::endl;
        }

        Imgui::panelEnd();
        Imgui::end();

        glfwSwapBuffers(window);
    }
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
