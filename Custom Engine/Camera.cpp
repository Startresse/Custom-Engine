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

void Camera::translate(glm::vec3 v)
{
    position_v += v;
    target_v += v;

    recalculate();
}

void Camera::zoom(double forward)
{
    assert(0.f <= zoom_speed && zoom_speed <= 1.f);

    position_v += static_cast<float>(forward) * (position_v - target_v) * zoom_speed;

    recalculate();
}
