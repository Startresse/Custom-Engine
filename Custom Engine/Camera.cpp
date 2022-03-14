#include "Camera.h"

void Camera::recalculate()
{
    direction_v = glm::normalize(position_v - target_v);
    right_v = glm::normalize(glm::cross(Direction::up, direction_v));
    up_v = glm::normalize(glm::cross(direction_v, right_v));

    view_mat = glm::lookAt(position_v, target_v, up_v);
}

void Camera::set_position(glm::vec3 pos)
{
    position_v = pos;
    recalculate();
}

void Camera::set_target(glm::vec3 pos)
{
    target_v = pos;
    recalculate();
}
