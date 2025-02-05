#include <iostream>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <cmath>

#include "ECS/ECS.h"
#include "ECS/Systems/Systems.h"
#include "Renderer/Renderer.h"
#include "Renderer/Shaders.h"
#include "Geometry.h"

int main (int argc, char *argv[])
{
    auto window = initializeOpenGLAndCreateWindow();
    if (not window)
        return -1;

    auto shaderProgram = createShaderProgram(vertexShaderSource, fragmentShaderSource);
     
    VBOData circleVBO = bufferData(createCircleVertices(0.02f, 20));
    auto VAO = createPosVAO(circleVBO.VBO);
    
    Registry registry;
    RenderSystem renderSystem;
    WoodCuttingSystem woodCuttingSystem;

    Entity worker = registry.create();
    Entity worker2 = registry.create();
    using Color = glm::vec4;
    using Pos = glm::vec2;

    registry.insert<Pos>(worker, Pos{-0.0, 0.0});
    registry.insert<Color>(worker, Color{ 0.2, 1.0, 0.2, 1.0 });
    registry.insert<RenderData>(worker, {VAO, circleVBO.vertexCount, GL_TRIANGLE_FAN});

    registry.insert<Pos>(worker2, Pos{-0.0, 0.0});
    registry.insert<Color>(worker2, Color{ 0.2, 1.0, 0.2, 1.0 });

    std::cout << "Entities: ";
    auto allEntities = registry.all<Pos, Color>();
    for (auto entity : allEntities)
    {
        std::cout << entity;
    }
    std::cout << std::endl;


    renderSystem.worker = worker;
    renderSystem.shaderID = shaderProgram;
    woodCuttingSystem.worker = worker;

    auto previousFrame = 0.f;
    while(!glfwWindowShouldClose(window))
    {
        auto currentFrame = glfwGetTime();
        auto timeDelta = currentFrame - previousFrame;
        previousFrame = currentFrame;
        processInput(window);

        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        glUseProgram(shaderProgram);
        woodCuttingSystem.run(registry, timeDelta);
        renderSystem.run(registry, timeDelta);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &circleVBO.VBO);
    glDeleteProgram(shaderProgram);

    glfwTerminate();
    return 0;
}
