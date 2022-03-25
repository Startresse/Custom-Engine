//! \file tuto7_camera.cpp reprise de tuto7.cpp mais en derivant AppCamera, avec gestion automatique d'une camera.

#include <cstring>

#include "app_camera.h"
#include "draw.h"        
#include "orbiter.h"
#include "program.h"
#include "texture.h"
#include "uniforms.h"
#include "vec.h"
#include "wavefront.h"

#include <map>

class Project : public AppCamera
{
public:
    Project( ) : AppCamera(1024, 640) {}
    
    int init( )
    {
    /* MESHES */
        m_light = read_mesh("data/sun.obj");

        m_objet = read_mesh("data/cube.obj");
        // m_objet = read_mesh("data/bigguy.obj");

        // m_plan = read_mesh("data/landscape.obj");
        m_plan = read_mesh("data/giant_plane.obj");

    /* CAMERA */
        Point pmin, pmax;
        m_plan.bounds(pmin, pmax);
        camera().lookat(pmin/2, pmax/2);

    /* SHADERS */
        object_shader = read_program("project/object_shader.glsl"); 
        program_print_errors(object_shader);
        light_shader = read_program("project/light_shader.glsl"); 
        program_print_errors(light_shader);
        shadow_map_shader = read_program("project/shadow_map_shader.glsl"); 
        program_print_errors(shadow_map_shader);

    /* SHADOW MAP CONFIGURATION */
        glGenFramebuffers(1, &depth_map_FBO);

        glGenTextures(1, &depth_map);
        glBindTexture(GL_TEXTURE_2D, depth_map);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, 
                    SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT,
                    GL_FLOAT, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER); 
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER); 
        float borderColor[] = {1.0, 1.0, 1.0, 1.0};
        glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);

        glBindFramebuffer(GL_FRAMEBUFFER, depth_map_FBO);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
                               GL_TEXTURE_2D, depth_map, 0);
        glDrawBuffer(GL_NONE);
        glReadBuffer(GL_NONE);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);

    /* OTHER SETTINGS */
        glClearColor(0.2f, 0.2f, 0.2f, 1.f);

        glClearDepth(1.f);        // profondeur par defaut
        glDepthFunc(GL_LESS);     // ztest, conserver l'intersection la plus proche de la camera
        glEnable(GL_DEPTH_TEST);  // activer le ztest

        return 0;
    }

    int quit( )
    {
        release_program(object_shader);
        release_program(light_shader);
        release_program(shadow_map_shader);
        m_light.release();
        m_objet.release();
        m_plan.release();
        return 0;
    }

    int render( )
    {

    /* GAME LOGIC */
        update_object_transform();

        update_light_transform();

    /* RENDERING */
        view = m_camera.view();
        projection = m_camera.projection(window_width(),
                                                   window_height(), 45);
        Transform model;

    /* FIRST PASS */
        Point light_pos = light_transform(Point(0, 0, 0));
        float near_plane = 50.0f, far_plane = 150.0f;
        // change magic number with pmin, pmax?
        Transform light_projection = Ortho(-30.0f, 30.0f, -30.0f, 30.0f,
                                          near_plane, far_plane);
        Transform light_view = Lookat(light_pos, 
                                  vec3( 0.0f, 0.0f,  0.0f), 
                                  vec3( 0.0f, 1.0f,  0.0f));  
        Transform light_space_matrix = light_projection * light_view;

        glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
        glBindFramebuffer(GL_FRAMEBUFFER, depth_map_FBO);
        glClear(GL_DEPTH_BUFFER_BIT);

        // depthMap shader
        glUseProgram(shadow_map_shader);
        program_uniform(shadow_map_shader, "light_space_matrix",
                        light_space_matrix);
        render_scene(shadow_map_shader, 0);

    /* SECOND PASS */
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glViewport(0, 0, window_width(), window_height());
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        /* LIGHT */
        glUseProgram(light_shader);
        program_uniform(light_shader, "mvp", projection * view * light_transform);
        program_uniform(light_shader, "light_color", light_color);
        m_light.draw(light_shader, true, false, false, false);

        /* OBJECTS */
        glUseProgram(object_shader);

        program_uniform(object_shader, "view", view);
        program_uniform(object_shader, "projection", projection);
        program_uniform(object_shader, "light_pos", light_pos);
        program_uniform(object_shader, "light_color", light_color);
        program_uniform(object_shader, "light_space_matrix", light_space_matrix);

        render_scene(object_shader, 1);

        frame++;
        return 1;
    }

protected:
/* MESHES */
    Mesh m_objet;
    Mesh m_light;
    Mesh m_plan;

/* SCENE */
    int frame = 0;

/* TRANSFORMS */
    Transform projection;
    Transform view;

/* SHADERS */
    // display shaders
    GLuint object_shader;
    GLuint light_shader;

    // shadow shader
    const unsigned int SHADOW_WIDTH = 1024, SHADOW_HEIGHT = 1024;
    GLuint shadow_map_shader;
    GLuint depth_map;
    GLuint depth_map_FBO;

/* PLANE */
    Transform plane_transform = Identity();
    vec3 plan_color = vec3(0.8f, 0.8f, 0.8f);

/* OBJECT */
    Transform object_transform = Translation(0, 2, 0);
    float object_rotation_speed = 1;
    float object_move_speed = .03f;
    vec3 object_color = vec3(1, 0.2, 0.31);

/* LIGHT */
    Transform light_transform;
    int nbr_frame_cycle = 200;
    float dist_light = 100;
    vec3 light_color = vec3(1, 1, 1);

/* CONTROLS */
    std::map<std::string, SDL_Keycode> controls = {
        {"inverse_key", SDLK_LSHIFT},
        {"obj_forward", 'w'},
        {"obj_back", 's'},
        {"obj_right", 'd'},
        {"obj_left", 'a'},
        {"obj_up", SDLK_SPACE},
        {"stop_light", 'p'}
    };

    
private:

    // TODO include speed softening
    void update_object_transform() {
        Transform position = Identity();
        if (key_state(controls["obj_forward"])) {
            position = position * Translation(object_move_speed, 0.0, 0.0);
        }
        if (key_state(controls["obj_back"])) {
            position = position * Translation(-object_move_speed, 0.0, 0.0);
        }
        if (key_state(controls["obj_up"]) &&
            !key_state(controls["inverse_key"])) {
            position = position * Translation(0.0, object_move_speed, 0.0);
        }
        if (key_state(controls["obj_up"]) &&
            key_state(controls["inverse_key"])) {
            position = position * Translation(0.0, -object_move_speed, 0.0);
        }
        if (key_state(controls["obj_left"])) {
            position = RotationY(object_rotation_speed) * position;
        }
        if (key_state(controls["obj_right"])) {
            position = RotationY(-object_rotation_speed) * position;
        }

        object_transform = object_transform * position;
    }

    void update_light_transform() {
        static bool pressed = false;
        static bool light_stop = false;
        if (key_state(controls["stop_light"]) && !pressed) {
            pressed = true;
            light_stop = !light_stop;
        }
        if (!key_state(controls["stop_light"])) {
            pressed = false;
        }
        if (light_stop) {
            frame--;
            return;
        }
        float day_time = static_cast<float>(frame) /
                         static_cast<float>(nbr_frame_cycle);
        float pos_x = cos(day_time);
        float pos_y = sin(day_time);
        light_transform = Translation(dist_light * pos_x,
                                      dist_light * pos_y,
                                      0.0f);
        float threshold = 1;
        if (pos_y > threshold) {
            light_color = vec3(1.0, 1.0, 1.0);
        } else {
            float red_factor = 0.2;
            float blue_factor = 1.3;
            float green_factor = 1.7;
            float red = std::max(0.4,
                1.0 - threshold * red_factor + red_factor * pos_y);
            float blue = std::max(0.4,
                1.0 - threshold * blue_factor + blue_factor * pos_y);
            float green = std::max(0.4,
                1.0 - threshold * green_factor + green_factor * pos_y);
            light_color = vec3(red, blue, green);
        }
    }

    void render_scene(GLuint shader, bool color) {
        /* OBJECT */
        Transform model = object_transform;

        program_uniform(shader, "model", model);
        if (color)
            program_uniform(shader, "object_color", object_color);

        m_objet.draw(shader, true, false, true, false);

        /* FLOOR */
        model = plane_transform;

        program_uniform(shader, "model", model);
        if (color)
            program_uniform(shader, "object_color", plan_color);

        m_plan.draw(shader, true, false, true, false);
    }

};

int main( int argc, char **argv )
{
    Project tp;
    tp.run();

    return 0;
}
