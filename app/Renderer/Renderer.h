#ifndef RENDERER_RENDERER_H
#define RENDERER_RENDERER_H

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <string>
#include <iostream>
#include <cstddef>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

enum class DrawStrategy
{
    ARRAYS,
    ELEMENTS
};

struct RenderData
{
    unsigned int VAO = 0;
    size_t elementCount = 0;
    int drawMode = GL_TRIANGLES;
    DrawStrategy drawStrategy = DrawStrategy::ARRAYS;
};

struct BufferData {
    unsigned int handle = 0;
    size_t elementCount = 0;
};

unsigned int getLoc(unsigned int shader, const std::string& name)
{
    return glGetUniformLocation(shader, name.c_str());
}

void setUniform(unsigned int shader, const std::string& name, const glm::vec4& vec)
{
    glUniform4f(getLoc(shader, name), vec.x, vec.y, vec.z, vec.w);
}

void setUniform(unsigned int shader, const std::string& name, const glm::mat4& mat)
{
    glUniformMatrix4fv(getLoc(shader, name), 1, GL_FALSE, glm::value_ptr(mat));
}

void render(const RenderData& renderData)
{
    glBindVertexArray(renderData.VAO);
    if (renderData.drawStrategy == DrawStrategy::ELEMENTS)
    {
        glDrawElements(renderData.drawMode, renderData.elementCount, GL_UNSIGNED_INT, nullptr);
    }
    else
    {
        glDrawArrays(renderData.drawMode, 0, renderData.elementCount);
    }
}

[[nodiscard]] BufferData bufferData(const std::vector<glm::vec2>& data) 
{
    unsigned int vbo;
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, data.size() * sizeof(glm::vec2), &data[0], GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    return {vbo, data.size()};
}

[[nodiscard]] BufferData bufferIndexData(const std::vector<unsigned int>& data)
{
    unsigned int ebo;
    glGenBuffers(1, &ebo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, data.size() * sizeof(unsigned int), &data[0], GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    return {ebo, data.size()};
}

[[nodiscard]] unsigned int createPosVAO(unsigned int posVBO, unsigned int ebo = 0)
{
    unsigned int VAO;
    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
    glVertexArrayVertexBuffer(VAO, 0, posVBO, 0, 2 * sizeof(float));
    glEnableVertexAttribArray(0);
    if (ebo > 0)
    {
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    }
    glBindVertexArray(0);
    return VAO;
}

[[nodiscard]] unsigned int createShaderProgram(const char* vertexShaderSource, const char* fragmentShaderSource)
{
    unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, nullptr);
    glCompileShader(vertexShader);
    
    unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, nullptr);
    glCompileShader(fragmentShader);

    unsigned int shaderProgram;
    shaderProgram = glCreateProgram();

    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
    return shaderProgram;
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}

void processInput(GLFWwindow* window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
    {
        glfwSetWindowShouldClose(window, true);
    }
}

bool isKeyPressed(GLFWwindow* window, int key)
{
    return glfwGetKey(window, key) == GLFW_PRESS;
}

[[nodiscard]] GLFWwindow* initializeOpenGLAndCreateWindow()
{
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    
    GLFWwindow* window = glfwCreateWindow(800, 600, "LearnOpenGL", nullptr, nullptr);
    if (window == nullptr)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return nullptr;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std:: endl;
        return nullptr;
    }
    return window;
}

#endif
