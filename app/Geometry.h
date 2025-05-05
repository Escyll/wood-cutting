#pragma once 

#include <vector>
#include <glm/glm.hpp>

/// Returns circle with radius and with segmentCount segments at (0;0)
/// (0;0) first followed by edge vertices CCW
std::vector<glm::vec2> createCircleVertices(float radius, int segmentCount);

/// Returns rectangle from (0;0) to (width;height)
/// (0;0) first then CCW
std::vector<glm::vec2> createRectangleVertices(float width, float height);