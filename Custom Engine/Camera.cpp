#include "Camera.h"

void Camera::recalculate()
{
    direction_v = glm::normalize(position_v - target_v);
    // TODO change direction::up with quaternions
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

void Camera::rotate_around_target(float angle, glm::vec3 axis)
{
    // TODO update with quaternion
    glm::quat q = glm::angleAxis(angle, glm::normalize(axis));
    position_v = q * (position_v - target_v) + target_v;

    recalculate();
}
//
//void Camera::rotate_around_position(float angle, glm::vec3 axis)
//{
//    glm::quat q = glm::quat(glm::normalize(axis) * angle);
//    target_v = q * (target_v - position_v) + position_v;
//
//    recalculate();
//}
