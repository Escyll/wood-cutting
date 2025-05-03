#include "Renderer/Camera.h"

#include <glm/gtc/matrix_transform.hpp>

namespace Render {
    glm::mat4 createProjection(const Size2D& resolution, float maxFOV, float near, float far)
    {
        auto ratio = static_cast<float>(resolution.width) / resolution.height;
        if (resolution.width > resolution.height)
        {
            return glm::ortho(0.f, maxFOV, 0.f, maxFOV / ratio, near, far);
        }
        else
        {
            return glm::ortho(0.f, maxFOV * ratio, maxFOV, 0.f, near, far);
        }
    }
}
