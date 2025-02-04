#ifndef GEOMETRY_H
#define GEOMETRY_H

std::vector<glm::vec2> createCircleVertices(float radius, int segments)
{
    std::vector<glm::vec2> result;
    result.push_back({0.f, 0.f});
    for (int i = 0; i <= segments; i++)
    {
        auto angle = 2.f * M_PI * i / segments;
        result.push_back({radius*std::cos(angle), radius*std::sin(angle)});
    }
    return result;
}

#endif
