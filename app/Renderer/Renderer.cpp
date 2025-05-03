#include "Renderer/Renderer.h"

#include <iostream>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

bool operator==(const RenderData& renderDataA, const RenderData& renderDataB)
{
    return renderDataA.VAO == renderDataB.VAO &&
        renderDataA.posVBO == renderDataB.posVBO &&
        renderDataA.texVBO == renderDataB.texVBO &&
        renderDataA.drawMode == renderDataB.drawMode;
}

unsigned int getLoc(unsigned int shader, const std::string& name)
{
    return glGetUniformLocation(shader, name.c_str());
}

void setUniform(unsigned int shader, const std::string& name, int value)
{
    glUniform1i(getLoc(shader, name), value);
}

void setUniform(unsigned int shader, const std::string& name, const glm::vec4& vec)
{
    glUniform4f(getLoc(shader, name), vec.x, vec.y, vec.z, vec.w);
}

void setUniform(unsigned int shader, const std::string& name, const glm::mat4& mat)
{
    glUniformMatrix4fv(getLoc(shader, name), 1, GL_FALSE, glm::value_ptr(mat));
}

void renderText(const std::string& text, BMFont& font, unsigned int shader, unsigned int texture, RenderData& renderData)
{
    glm::vec2 imageScale { 1.f / font.common.scaleW, 1.f / font.common.scaleH };
    float xadvance = 0;
    float yadvance = 0;
    Render::printGLDebug("Rendering letters");
    Render::Material material;
    material.name = "Letter";
    material.shader = shader;
    material.uniformMatrix4fvs["view"] = glm::mat4(1.0f);
    material.renderData = renderData;
    material.texture = texture;
    Render::setMaterial(material);
    for (char letter : text)
    {
        if (letter == '\n')
        {
            yadvance += font.common.lineHeight;
            xadvance = 0;
            continue;
        }
        BMFontChar& ch = font.chars[letter];
        auto sh = font.common.scaleH;
        std::vector<glm::vec2> texCoords {
            imageScale*glm::vec2{ch.x + 1, sh - (ch.y + 1)},
            imageScale*glm::vec2{ch.x+ch.width - 1, sh - (ch.y + 1)},
            imageScale*glm::vec2{ch.x+ch.width - 1, sh - (ch.y + ch.height - 1)},
            imageScale*glm::vec2{ch.x+ch.width - 1, sh - (ch.y + ch.height - 1)},
            imageScale*glm::vec2{ch.x + 1, sh - (ch.y + ch.height - 1)},
            imageScale*glm::vec2{ch.x + 1, sh - (ch.y + 1)}
        };
        
        auto model = glm::translate(glm::mat4(1.0f), glm::vec3(xadvance + ch.xoffset, yadvance + ch.yoffset, 0.f));
        model = glm::scale(model, glm::vec3((float)ch.width / font.common.lineHeight, (float)ch.height / font.common.lineHeight, 1.0));
        auto posX = (float) xadvance + ch.xoffset;
        auto posY = (float) yadvance + ch.yoffset;
        auto width = (float)ch.width ;
        auto height = (float)ch.height;
        std::vector<glm::vec2> pos { {posX, posY}, {posX + width, posY}, {posX + width, posY + height}, {posX + width, posY + height}, {posX, posY + height}, {posX, posY}};
        Render::queue(pos, texCoords);
        xadvance += ch.xadvance;
    }
    Render::flush();
}

[[nodiscard]] BufferData bufferData(const std::vector<glm::vec2>& data) 
{
    unsigned int vbo;
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, data.size() * sizeof(glm::vec2), &data[0], GL_DYNAMIC_DRAW);
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

[[nodiscard]] unsigned int createPosVAO(unsigned int posVBO, unsigned int ebo)
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

[[nodiscard]] unsigned int createPosTexVAO(unsigned int posVBO, unsigned int texVBO, unsigned int ebo)
{
    unsigned int VAO;
    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(glm::vec2), (void*)0);
    glVertexArrayVertexBuffer(VAO, 0, posVBO, 0, sizeof(glm::vec2));
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(glm::vec2), (void*)0);
    glVertexArrayVertexBuffer(VAO, 1, texVBO, 0, sizeof(glm::vec2));
    glEnableVertexAttribArray(1);
    if (ebo > 0)
    {
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    }
    glBindVertexArray(0);
    return VAO;
}

namespace Render
{

RenderContext renderContext;

bool operator==(const Material& matA, const Material& matB)
{
    return matA.name == matB.name &&
        matA.shader == matB.shader &&
        matA.uniform1is == matB.uniform1is &&
        matA.uniform4fs == matB.uniform4fs &&
        matA.uniformMatrix4fvs == matB.uniformMatrix4fvs &&
        matA.renderData == matB.renderData &&
        matA.texture == matB.texture;
}

void setLayer(int layer)
{
    renderContext.activeLayer = layer;
    renderContext.activeSubLayer = 0;
    renderContext.activeMaterial = {};
}

void setSubLayer(float subLayer)
{
    renderContext.activeSubLayer = subLayer;
    renderContext.activeMaterial = {};
}

void setMaterial(const Material& material)
{
    renderContext.activeMaterial = material;
    renderContext.layers[renderContext.activeLayer].subLayers[renderContext.activeSubLayer].materials[material.name] = material;
}

void setCamera(Camera* camera)
{
    renderContext.activeCamera = camera;
}

void queue(const std::vector<glm::vec2>& newPositions)
{
    auto& subLayer = renderContext.layers[renderContext.activeLayer].subLayers[renderContext.activeSubLayer];
    auto& positions = subLayer.positions[renderContext.activeMaterial.name];
    positions.insert(positions.end(), newPositions.begin(), newPositions.end());
}

void queue(const std::vector<glm::vec2>& newPositions, const std::vector<glm::vec2>& newTexCoords)
{
    auto& subLayer = renderContext.layers[renderContext.activeLayer].subLayers[renderContext.activeSubLayer];
    auto& positions = subLayer.positions[renderContext.activeMaterial.name];
    positions.insert(positions.end(), newPositions.begin(), newPositions.end());
    auto& texCoords = subLayer.texCoords[renderContext.activeMaterial.name];
    texCoords.insert(texCoords.end(), newTexCoords.begin(), newTexCoords.end());
}

void printGLDebug(const std::string& message)
{
    glDebugMessageInsert(GL_DEBUG_SOURCE_APPLICATION, GL_DEBUG_TYPE_MARKER, 0, GL_DEBUG_SEVERITY_NOTIFICATION, -1, message.c_str());
}

void flush()
{
    auto camera = renderContext.activeCamera;
    if (not camera)
    {
        std::cerr << "No active camera, can't render" << std::endl;
        return;
    }
    for (auto& [layerNumber, layer] : renderContext.layers)
    {
        printGLDebug(std::string("Layer ") + std::to_string(layerNumber));
        for (auto& [subLayerNumber, subLayer] : layer.subLayers)
        {
            printGLDebug(std::string("SubLayer ") + std::to_string(subLayerNumber));
            for (auto& [materialName, material] : subLayer.materials)
            {
                printGLDebug(std::string("Material ") + materialName);
                glUseProgram(material.shader);
                setUniform(material.shader, "projection", camera->projection);
                glm::vec3 camPos3 { camera->position.x, camera->position.y, 0.f };
                auto view = glm::translate(glm::mat4(1.0f), -camPos3);
                setUniform(material.shader, "view", view);
                for (auto& [uniformName, value] : material.uniform1is)
                {
                    setUniform(material.shader, uniformName, value);
                }
                for (auto& [uniformName, value] : material.uniform4fs)
                {
                    setUniform(material.shader, uniformName, value);
                }
                for (auto& [uniformName, value] : material.uniformMatrix4fvs)
                {
                    setUniform(material.shader, uniformName, value);
                }
                
                glBindVertexArray(material.renderData.VAO);

                glActiveTexture(GL_TEXTURE0);
                glBindTexture(GL_TEXTURE_2D, material.texture);
                
                auto& positions = subLayer.positions[material.name];
                glBindBuffer(GL_ARRAY_BUFFER, material.renderData.posVBO);
                glBufferData(GL_ARRAY_BUFFER, positions.size()*sizeof(glm::vec2), &positions[0], GL_DYNAMIC_DRAW);
                
                auto& texCoords = subLayer.texCoords[material.name];
                glBindBuffer(GL_ARRAY_BUFFER, material.renderData.texVBO);
                glBufferData(GL_ARRAY_BUFFER, texCoords.size()*sizeof(glm::vec2), &texCoords[0], GL_DYNAMIC_DRAW);
                
                glDrawArrays(material.renderData.drawMode, 0, positions.size());
            }
        }
    }
    renderContext.layers = {};
}

}
