#define _USE_MATH_DEFINES

#include "Geometry.h"

#include <cmath> 

/// Returns circle with radius and with segmentCount segments at (0;0)
/// (0;0) first followed by edge vertices CCW
std::vector<glm::vec2> createCircleVertices(float radius, int segmentCount)
{
    std::vector<glm::vec2> result;
    result.push_back({ 0.f, 0.f });
    for (int i = 0; i <= segmentCount; i++)
    {
        auto angle = 2.f * M_PI * i / segmentCount;
        result.push_back({ radius * std::cos(angle), radius * std::sin(angle) });
    }
    return result;
}

/// Returns rectangle from (0;0) to (width;height)
/// (0;0) first then CCW
std::vector<glm::vec2> createRectangleVertices(float width, float height)
{
    return { {0, 0}, {width, 0}, {width, height}, {0, height} };
}