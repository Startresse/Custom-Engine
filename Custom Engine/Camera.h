#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "Direction.h"

class Camera
{
public:

    /* CONSTRUCTORS */

    Camera() : Camera(default_position) {}
    Camera(glm::vec3 position_) : Camera(position_, default_target) {}
    Camera(glm::vec3 position_, glm::vec3 target_) :
        position_v(position_), target_v(target_)
    {
        init();
    }

    /* INFORMATIONS */

    glm::vec3 direction() const { return glm::normalize(position_v - target_v); }

    float distance_to_target() const { return glm::length(position_v - target_v); }


    /* GETTERS */

    glm::mat4 view() const { return view_mat; }
    glm::mat4 projection() const { return projection_mat; }

    glm::vec3 position() const { return position_v; }
    glm::vec3 target() const { return target_v; }
    glm::vec3 up() const { return up_v; }
    glm::vec3 right() const { return glm::normalize(glm::cross(up_v, direction())); }


    /* SETTERS */

    glm::mat4& view() { return view_mat; }
    glm::mat4& projection() { return projection_mat; }

    void set_projection(glm::mat4 projection) { projection_mat = projection; }

    void set_position(glm::vec3 pos);
    void set_target(glm::vec3 pos);
    void set_up(glm::vec3 pos);

    void set_zoom_speed(float new_speed_);

    /* MOVEMENT */

    void translate(glm::vec3 v);
    void rotate_around_target(float angle, glm::vec3 axis = Direction::up);
    void rotate_around_position(float angle, glm::vec3 axis = Direction::up);

    void zoom(double forward);


    /* STATICS VALUES */

    static constexpr glm::vec3 default_position = 2.f * Direction::up + 3.f * Direction::forward;
    static constexpr glm::vec3 default_target = glm::vec3(0.f);

private:
    glm::vec3 position_v;
    glm::vec3 target_v;
    glm::vec3 up_v;

    glm::mat4 view_mat;
    glm::mat4 projection_mat;

    // zoom speed is fraction sweeped each wheel click
    float zoom_speed = 0.1f;


    /* INTERNAL CALCULATIONS */

    void recalculate();
    void init();

};

