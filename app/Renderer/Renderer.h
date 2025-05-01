#pragma once

#include <string>
#include <glad/glad.h>
#include <glm/glm.hpp>

#include "FontRendering/BMFont.h"

struct RenderData
{
    unsigned int VAO = 0;
    unsigned int posVBO = 0;
    unsigned int texVBO = 0;
    int drawMode = GL_TRIANGLES;
};

bool operator==(const RenderData& renderDataA, const RenderData& renderDataB);

struct BufferData {
    unsigned int handle = 0;
    size_t elementCount = 0;
};

unsigned int getLoc(unsigned int shader, const std::string& name);
void setUniform(unsigned int shader, const std::string& name, int value);
void setUniform(unsigned int shader, const std::string& name, const glm::vec4& vec);
void setUniform(unsigned int shader, const std::string& name, const glm::mat4& mat);
void renderText(const std::string& text, BMFont& font, unsigned int shader, unsigned int texture, RenderData& renderData);

[[nodiscard]] BufferData bufferData(const std::vector<glm::vec2>& data) ;
[[nodiscard]] BufferData bufferIndexData(const std::vector<unsigned int>& data);
[[nodiscard]] unsigned int createPosVAO(unsigned int posVBO, unsigned int ebo = 0);
[[nodiscard]] unsigned int createPosTexVAO(unsigned int posVBO, unsigned int texVBO, unsigned int ebo = 0);

namespace Render
{

struct Material
{
    std::string name;
    unsigned int shader;
    std::map<std::string, int> uniform1is;
    std::map<std::string, glm::vec4> uniform4fs;
    std::map<std::string, glm::mat4> uniformMatrix4fvs;
    RenderData renderData;
    unsigned int texture;
};

bool operator==(const Material& matA, const Material& matB);

struct SubLayer
{
    std::unordered_map<std::string, std::vector<glm::vec2>> positions;
    std::unordered_map<std::string, std::vector<glm::vec2>> texCoords;
    std::unordered_map<std::string, Material> materials;
};

struct Layer
{
    std::map<float, SubLayer> subLayers;
};

struct RenderContext
{
    std::map<int, Layer> layers;
    int activeLayer = 0;
    float activeSubLayer = 0;
    Material activeMaterial = {};
};

void setLayer(int layer);
void setSubLayer(float subLayer);
void setMaterial(const Material& material);
void queue(const std::vector<glm::vec2>& newPositions);
void queue(const std::vector<glm::vec2>& newPositions, const std::vector<glm::vec2>& newTexCoords);
void printGLDebug(const std::string& message);
void flush();

}
