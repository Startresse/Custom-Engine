#include "Camera.h"

/* INTERNAL CALCULATIONS */

void Camera::init()
{
    glm::vec3 direction_v = glm::normalize(position_v - target_v);
    glm::vec3 right_v = glm::normalize(glm::cross(Direction::up, direction_v));

    up_v = glm::normalize(glm::cross(direction_v, right_v));

    recalculate();
}

void Camera::recalculate()
{
    view_mat = glm::lookAt(position_v, target_v, up_v);
}


/* SETTERS */

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

void Camera::set_up(glm::vec3 up)
{
    up_v = up;
    recalculate();
}

void Camera::set_zoom_speed(float s)
{
    assert(0.f <= s && s <= 1.f);
    zoom_speed = s;
}

/* MOVEMENT */

void Camera::translate(glm::vec3 v)
{
    position_v += v;
    target_v += v;

    recalculate();
}

void Camera::zoom(double forward)
{
    position_v += static_cast<float>(forward) * (position_v - target_v) * zoom_speed;

    recalculate();
}

void Camera::rotate_around_target(float angle, glm::vec3 axis)
{
    assert(glm::length(axis) > 0);

    glm::quat q = glm::angleAxis(angle, glm::normalize(axis));
    position_v = q * (position_v - target_v) + target_v;
    up_v = q * up_v;

    recalculate();
}

void Camera::rotate_around_position(float angle, glm::vec3 axis)
{
    assert(glm::length(axis) > 0);

    glm::quat q = glm::angleAxis(angle, glm::normalize(axis));
    target_v = q * (target_v - position_v) + position_v;
    up_v = q * up_v;

    recalculate();
}
