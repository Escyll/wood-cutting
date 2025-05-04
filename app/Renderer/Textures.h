#pragma once

#include <string>
#include <vector>
#include <glm/glm.hpp>

enum TEXTURE_FILTER
{
    LINEAR,
    NEAREST
};

bool loadTexture(const std::string& path, unsigned int& texture, TEXTURE_FILTER filter);

struct TextureRegion {
    glm::vec2 bottomLeft;
    glm::vec2 size;
};

struct Frame {
    TextureRegion textureRegion;
    float duration;
};

struct AnimationSequence {
    std::string name;
    std::string texture;
    float duration;
    std::vector<Frame> frames;
};

struct AnimationState {
    std::string animation;
    float elapsedTime;
    Frame currentFrame;
};
