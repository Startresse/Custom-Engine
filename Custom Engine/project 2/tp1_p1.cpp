#include "app.h"
#include "app_time.h"
#include "draw.h"
#include "orbiter.h"
#include "program.h"
#include "texture.h"
#include "uniforms.h"
#include "wavefront.h"

class TP : public AppTime
{
public:
    // constructeur : donner les dimensions de l'image, et eventuellement la version d'openGL.
    TP() : AppTime(1024, 640) {}

    // creation des objets de l'application
    int init()
    {
        m_vect.push_back(read_mesh("data/quaternius/Street_3Way.obj"));
        m_vect.push_back(read_mesh("data/quaternius/Street_3Way_2.obj"));
        m_vect.push_back(read_mesh("data/quaternius/Street_4Way.obj"));
        m_vect.push_back(read_mesh("data/quaternius/Street_4Way_2.obj"));
        m_vect.push_back(read_mesh("data/quaternius/Street_Bridge.obj"));
        m_vect.push_back(read_mesh("data/quaternius/Street_Bridge_Ramp.obj"));
        m_vect.push_back(read_mesh("data/quaternius/Street_Bridge_Underpass.obj"));
        m_vect.push_back(read_mesh("data/quaternius/Street_Bridge_Water.obj"));
        m_vect.push_back(read_mesh("data/quaternius/Street_Bridge_WaterRamp.obj"));
        m_vect.push_back(read_mesh("data/quaternius/Street_Curve.obj"));
        m_vect.push_back(read_mesh("data/quaternius/Street_Deadend.obj"));
        m_vect.push_back(read_mesh("data/quaternius/Street_Elevated.obj"));
        m_vect.push_back(read_mesh("data/quaternius/Street_Elevated_Ramp.obj"));
        m_vect.push_back(read_mesh("data/quaternius/Street_Empty.obj"));
        m_vect.push_back(read_mesh("data/quaternius/Street_Empty_Water.obj"));
        m_vect.push_back(read_mesh("data/quaternius/Street_Straight.obj"));

        for (Mesh m : m_vect)
        {
            Point local_pmin, local_pmax;
            m.bounds(local_pmin, local_pmax);
            pmin = min(pmin, local_pmin);
            pmax = max(pmax, local_pmax);
        }
        m_camera.lookat(distance_factor * nbr_rows * pmin, distance_factor * nbr_rows * pmax);

        m_program = read_program("tps/shaders/shader_1.glsl");
        program_print_errors(m_program);

        glClearColor(0.2f, 0.2f, 0.2f, 1.f);

        glClearDepth(2.f);
        glDepthFunc(GL_LESS);
        glEnable(GL_DEPTH_TEST);

        return 0;
    }

    int quit()
    {
        release_program(m_program);

        for (Mesh m : m_vect)
            m.release();

        return 0;
    }

    // dessiner une nouvelle image
    int render()
    {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // deplace la camera
        int mx, my;
        unsigned int mb = SDL_GetRelativeMouseState(&mx, &my);
        if (mb & SDL_BUTTON(1)) // le bouton gauche est enfonce
            m_camera.rotation(mx, my);
        else if (mb & SDL_BUTTON(3)) // le bouton droit est enfonce
            m_camera.move(mx);
        else if (mb & SDL_BUTTON(2)) // le bouton du milieu est enfonce
            m_camera.translation((float)mx / (float)window_width(), (float)my / (float)window_height());

        if (key_state('r'))
        {
            clear_key_state('r');
            reload_program(m_program, "tps/shaders/shader_1.glsl");
            program_print_errors(m_program);
        }

        // rendu

        glUseProgram(m_program);

        Transform view = m_camera.view();
        Transform projection = m_camera.projection(window_width(), window_height(), 45);

        program_uniform(m_program, "viewMatrix", view);
        program_uniform(m_program, "viewInvMatrix", view.inverse());
        program_uniform(m_program, "pos_light", distance_factor * nbr_rows * pmax);

        int mesh_nbr = 0;
        for (int trans_x = -nbr_rows + 1; trans_x <= nbr_rows - 1; trans_x++)
        {
            for (int trans_z = -nbr_rows + 1; trans_z <= nbr_rows - 1; trans_z++)
            {
                Transform translate = 
                    Translation(Vector(pmax, pmin).x * distance_factor * trans_x,
                                0,
                                Vector(pmax, pmin).z * distance_factor * trans_z);

                Transform model = translate * RotationY(global_time() / SPEED_FACTOR);
                Transform mvp = projection * view * model;
                program_uniform(m_program, "mvpMatrix", mvp);
                program_uniform(m_program, "modelMatrix", model);
                program_uniform(m_program, "indexMesh", mesh_nbr);
                m_vect[mesh_nbr].draw(m_program, true, false, true, false, false);
                mesh_nbr++;
                mesh_nbr = mesh_nbr % m_vect.size();
            }
        }

        return 1;
    }

protected:
    const float SPEED_FACTOR = 6000;

    std::vector<Mesh> m_vect;
    Orbiter m_camera;

    Point pmin, pmax;

    int nbr_rows = 5;
    float distance_factor = 1.5;

    GLuint m_program;
};

int main(int argc, char **argv)
{
    TP tp;
    tp.run();

    return 0;
}
