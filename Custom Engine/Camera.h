#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

const glm::vec3 WORLD_UP(0.0f, 1.0f, 0.0f);
    

class Camera
{
public:
    Camera() : Camera(glm::vec3(0.0f, 2.0f, 3.0f)) {}
    Camera(glm::vec3 position_) : Camera(position_, glm::vec3(0.0f, 0.0f, 0.0f)) {}
    Camera(glm::vec3 position_, glm::vec3 target_) :
        position_v(position_), target_v(target_)
    {
        recalculate();
    }

    void set_position(glm::vec3 pos);
    void set_target(glm::vec3 pos);
    void set_projection(glm::mat4 proj) { projection_mat = proj; }

    glm::mat4 view() const { return view_mat; }
    glm::mat4 projection() const { return projection_mat; }

    glm::vec3 position() const { return position_v; }
    glm::vec3 target() const { return target_v; }
    glm::vec3 direction() const { return direction_v; }
    glm::vec3 right() const { return right_v; }
    glm::vec3 up() const { return up_v; }

private:
    glm::vec3 position_v;
    glm::vec3 target_v;
    glm::vec3 direction_v;
    glm::vec3 right_v;
    glm::vec3 up_v;

    glm::mat4 view_mat;
    glm::mat4 projection_mat;

    void recalculate();

};

