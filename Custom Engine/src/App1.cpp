#include "App1.h"

/* CALLBACKS */

// Keyboard input
void keyboard_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    App1* app = static_cast<App1*>(glfwGetWindowUserPointer(window));

    // G + 1 : toggle grid
    if (key == GLFW_KEY_1 && action == GLFW_PRESS && glfwGetKey(window, GLFW_KEY_G))
        app->grid.toggle();

    // G + 2 : toggle axes
    if (key == GLFW_KEY_2 && action == GLFW_PRESS && glfwGetKey(window, GLFW_KEY_G))
        app->axes.toggle();

    // W : toggle wireframe
    if (key == GLFW_KEY_W && action == GLFW_PRESS)
    {
        GLint polygon_mode;
        glGetIntegerv(GL_POLYGON_MODE, &polygon_mode);
        if (polygon_mode == GL_LINE)
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        else
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    }
}

// Mouse input
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
    App1* app = static_cast<App1*>(glfwGetWindowUserPointer(window));
    glm::dvec2& a = app->last_mouse_pos;

    // Set last mouse position for draging mouse with wheel
    if (button == GLFW_MOUSE_BUTTON_MIDDLE && action == GLFW_PRESS)
    {
        glfwGetCursorPos(window, &(a.x), &(a.y));
        if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT))
            app->camera_movement = App1::CameraMovement::Translate;
        else
            app->camera_movement = App1::CameraMovement::Rotate;
    }
}

// Scroll wheel input
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    App1* app = static_cast<App1*>(glfwGetWindowUserPointer(window));

    // Scroll : zoom (forward) / unzoom (bakwards)
    app->camera.zoom(-yoffset);
}


/* APP1 */

int App1::handle_input()
{
    // Mouse middle button hold to rotate camera
    if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_MIDDLE))
    {
        glm::dvec2 current_mouse_pos;
        glfwGetCursorPos(window, &(current_mouse_pos.x), &(current_mouse_pos.y));

        glm::vec2 diff = last_mouse_pos - current_mouse_pos;
        // x = right, y = up
        diff.y = -diff.y;
        // diff represents percentage of window browsed
        diff.x = diff.x / window_width();
        diff.y = diff.y / window_height();

        switch (camera_movement)
        {
        case CameraMovement::Rotate:

            diff.x *= 4.f * static_cast<float>(std::numbers::pi);
            diff.y *= 2.f * static_cast<float>(std::numbers::pi);

            // rotate along x diff
            camera.rotate_around_target(diff.x, Direction::up);

            // rotate along y diff
            glm::vec3 rotation_axis = glm::normalize(glm::cross(camera.direction(), camera.up()));
            camera.rotate_around_target(diff.y, rotation_axis);

            break;
        case CameraMovement::Translate:

            glm::vec3 translation = diff.x * camera.right() + diff.y * camera.up();
            // The closer the target, the lesser the translation
            translation *= camera.distance_to_target();

            camera.translate(translation);

            break;
        default:
            break;
        }

        // update last
        glfwGetCursorPos(window, &(last_mouse_pos.x), &(last_mouse_pos.y));
    }

    return 0;
}

int App1::init()
{
    // Set input callbacks
    if (glfwRawMouseMotionSupported())
        glfwSetInputMode(window, GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);
    else
        std::cout << "WARNING : mouse raw motion not supported" << std::endl;

    glfwSetKeyCallback(window, keyboard_callback);
    glfwSetMouseButtonCallback(window, mouse_button_callback);
    glfwSetScrollCallback(window, scroll_callback);


    // Shader
    program = Shader("src/shaders/default_shader.glsl");

    // Meshes
    meshes.push_back(read_mesh("data/cube.obj"));
    //meshes.push_back(read_mesh("data/cornell.obj"));

    // Camera
    camera.set_target(glm::vec3(0.f, 0.f, 0.f));
    glm::vec4 camera_pos_init_h = glm::vec4(Direction::up + 3.f * Direction::forward, 1.f);
    camera.set_position(camera_pos_init_h);

    return 0;
}

int App1::render()
{
    //double fps = 1.f / (glfwGetTime() - last_time);
    //last_time = glfwGetTime();
    //std::cout << fps << " fps" << std::endl;

    Color c = Color::background_grey;
    glClearColor(c.x, c.y, c.z, c.w);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    //glm::vec4 camera_pos_init_h = glm::vec4(Direction::up + 3.f * Direction::forward, 1.f);
    //camera.set_position(glm::vec3(glm::rotate(glm::mat4(1.0), (float)glfwGetTime(), Direction::up) * camera_pos_init_h));
    //camera.translate(glm::vec3(0.005f, 0.f, 0.f));
    //camera.zoom(sin(glfwGetTime()) >= 0.f);
    //camera.rotate_around_target(glm::radians(90.f)/ 144.f, glm::vec3(1, 0.1, 0));
    //camera.rotate_around_position(glm::radians(90.f)/ 144.f, glm::vec3(1, 0.1, 0));

    glm::mat4 view = camera.view();
    glm::mat4 proj = camera.projection();
    glm::mat4 mvp = proj * view * glm::mat4(1.0);

    glm::mat4 trans = glm::mat4(1.0);
    //trans = glm::rotate(trans, (float)glfwGetTime(), glm::normalize(glm::vec3(0.0, 1.0, 0.0)));
    trans = glm::translate(trans, glm::vec3(-0.5f));
    //trans = glm::rotate(trans, 45.0f, glm::normalize(glm::vec3(0.5, 0.5, 0.0)));

    //axes.draw(proj * view * glm::translate(glm::mat4(1.f), camera.position() - camera.direction()));
    axes.draw(camera, *this);
    grid.draw(mvp);
    // grid won't display over anything
    //glClear(GL_DEPTH_BUFFER_BIT);

    // Models
    program.use();
    program.setMat4("model", trans);
    program.setMat4("view", view);
    program.setMat4("projection", proj);
    program.setVec3("light_pos", 10.f * camera.direction() + 3.0f * camera.up() - 2.0f * camera.right());

    for (auto& mesh : meshes)
        mesh.draw(program.ID);

    return 0;
}

int App1::quit()
{
    return 0;
}
