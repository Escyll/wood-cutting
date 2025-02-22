//#include <glad/glad.h>
//#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <cmath>

#include "ECS/ECS.h"
#include "ECS/Systems/Systems.h"
#include "Renderer/Renderer.h"
#include "Renderer/Shaders.h"
#include "Geometry.h"

int main(int argc, char *argv[])
{
    Registry registry;
    auto window = initializeOpenGLAndCreateWindow();
    if (not window)
        return -1;

    auto shaderProgram = createShaderProgram(vertexShaderSource, fragmentShaderSource);

    BufferData circleBuffer = bufferData(createCircleVertices(0.01f, 20));
    BufferData lumberMillBuffer = bufferData(createRectangleVertices(0.08f, 0.08f));
    BufferData barBuffer = bufferData(createRectangleVertices(0.01f, 0.2f));
    BufferData rectangleIndexBuffer = bufferIndexData({0,1,2,2,3,0});

    auto circleVAO = createPosVAO(circleBuffer.handle);
    auto lumberMillVAO = createPosVAO(lumberMillBuffer.handle, rectangleIndexBuffer.handle);
    auto barVAO = createPosVAO(barBuffer.handle, rectangleIndexBuffer.handle);

    RenderSystem renderSystem;
    renderSystem.shaderID = shaderProgram;
    WoodCuttingSystem woodCuttingSystem;
    PongMovementSystem pongMovementSystem;
    PongCollisionSystem pongCollisionSystem;

    Entity worker = registry.create();
    Entity worker2 = registry.create();
    Entity lumberMill = registry.create();
    Entity bar1 = registry.create();
    Entity bar2 = registry.create();
    Entity ball = registry.create();

    pongMovementSystem.bar1 = bar1;
    pongMovementSystem.bar2 = bar2;
    pongMovementSystem.window = window;
    pongCollisionSystem.bar1 = bar1;
    pongCollisionSystem.bar2 = bar2;
    pongCollisionSystem.ball = ball;

    using Color = glm::vec4;
    using Pos = glm::vec2;

    registry.insert<Pos>(worker, Pos{-0.4, 0.0});
    registry.insert<Patrol>(worker, Patrol{-0.5f, 0.5f, .5f, 1});
    registry.insert<Color>(worker, Color{0.2, 1.0, 0.2, 1.0});
    registry.insert<RenderData>(worker, {circleVAO, circleBuffer.elementCount, GL_TRIANGLE_FAN});

    registry.insert<Pos>(worker2, Pos{0.2, 0.3});
    registry.insert<Patrol>(worker2, Patrol{-0.5f, 0.5f, .5f, -1});
    registry.insert<Color>(worker2, Color{0.4, 0.4, 1.0, 1.0});
    registry.insert<RenderData>(worker2, {circleVAO, circleBuffer.elementCount, GL_TRIANGLE_FAN});

    registry.insert<Pos>(lumberMill, Pos{0.0, -0.8});
    registry.insert<Color>(lumberMill, Color{0.6, 0.2, 0.2, 1.0});
    registry.insert<RenderData>(lumberMill, {lumberMillVAO, rectangleIndexBuffer.elementCount, GL_TRIANGLES, DrawStrategy::ELEMENTS});

    registry.insert<Pos>(ball, Pos{-0.8, 0.0});
    registry.insert<Direction>(ball, Direction(1.0, 0.5));
    registry.insert<Color>(ball, Color{1.0, 1.0, 1.0, 1.0});
    registry.insert<RenderData>(ball, {circleVAO, circleBuffer.elementCount, GL_TRIANGLE_FAN});

    registry.insert<Pos>(bar1, Pos{-0.9, 0.0});
    registry.insert<Color>(bar1, Color{0.1, 0.1, 0.1, 1.0});
    registry.insert<RenderData>(bar1, {barVAO, rectangleIndexBuffer.elementCount, GL_TRIANGLES, DrawStrategy::ELEMENTS});

    registry.insert<Pos>(bar2, Pos{0.9, 0.0});
    registry.insert<Color>(bar2, Color{0.1, 0.1, 0.1, 1.0});
    registry.insert<RenderData>(bar2, {barVAO, rectangleIndexBuffer.elementCount, GL_TRIANGLES, DrawStrategy::ELEMENTS});

    glfwSwapInterval(1);

    auto previousFrame = 0.f;
    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();
        auto currentFrame = glfwGetTime();
        auto timeDelta = currentFrame - previousFrame;
        previousFrame = currentFrame;
        processInput(window);

        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        glUseProgram(shaderProgram);
        woodCuttingSystem.run(registry, timeDelta);
        renderSystem.run(registry, timeDelta);
        pongMovementSystem.run(registry, timeDelta);
        pongCollisionSystem.run(registry, timeDelta);

        glfwSwapBuffers(window);
    }
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
