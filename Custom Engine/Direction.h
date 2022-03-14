#pragma once

#include <glm/vec3.hpp>

namespace Direction
{
    static const glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);
    static const glm::vec3 down = -up;

    static const glm::vec3 right = glm::vec3(1.0f, 0.0f, 0.0f);
    static const glm::vec3 left = -right;

    static const glm::vec3 forward = glm::vec3(0.0f, 0.0f, 1.0f);
    static const glm::vec3 back = -forward;
}


