#pragma once

#include <glm/vec3.hpp>
#include <glm/glm.hpp>

#include <assert.h>

// if undefined, Diurection::up is along Y
//#define UP_IS_Z

namespace Direction
{
    static constexpr glm::vec3 right = glm::vec3(1.f, 0.f, 0.f);

#ifdef UP_IS_Z
    static constexpr glm::vec3 up = glm::vec3(0.f, 0.f, 1.f);
    static constexpr glm::vec3 forward = glm::vec3(0.f, 1.f, 0.f);
#else
    static constexpr glm::vec3 up = glm::vec3(0.f, 1.f, 0.f);
    static constexpr glm::vec3 forward = glm::vec3(0.f, 0.f, 1.f);
#endif

    static constexpr glm::vec3 left = -right;
    static constexpr glm::vec3 down = -up;
    static constexpr glm::vec3 back = -forward;
}
