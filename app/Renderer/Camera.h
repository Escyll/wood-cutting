#pragma once

#include <glm/glm.hpp>

namespace Render {
    struct Size2D {
        int width;
        int height;
    };

    struct Camera
    {
        glm::vec2 position;
        glm::mat4 projection;
    };
    
    glm::mat4 createProjection(const Size2D& resolution, float maxFOV, float near, float far);
}
