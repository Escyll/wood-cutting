#pragma once

#include <string>
#include <vector>
#include <glm/glm.hpp>

bool loadTexture(const std::string& path, unsigned int& texture);

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
