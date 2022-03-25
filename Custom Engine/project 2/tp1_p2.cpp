
//! \file tuto7.cpp reprise de tuto6.cpp mais en derivant App::init(), App::quit() et bien sur App::render().

#include "wavefront.h"

#include "orbiter.h"
#include "draw.h"        
#include "app.h"        // classe Application a deriver

#include "app_time.h"

#include "program.h"
#include "uniforms.h"


class TP : public AppTime
{
public:
    // constructeur : donner les dimensions de l'image, et eventuellement la version d'openGL.
    TP( ) : AppTime(1024, 640) {}
    
    // creation des objets de l'application
    int init( )
    {
        std::vector<const char*> object_names;
        object_names.push_back("data/quaternius/Street_3Way.obj");
        object_names.push_back("data/quaternius/Street_3Way_2.obj");
        object_names.push_back("data/quaternius/Street_4Way.obj");
        object_names.push_back("data/quaternius/Street_4Way_2.obj");
        object_names.push_back("data/quaternius/Street_Bridge.obj");
        object_names.push_back("data/quaternius/Street_Bridge_Ramp.obj");
        object_names.push_back("data/quaternius/Street_Bridge_Underpass.obj");
        object_names.push_back("data/quaternius/Street_Bridge_Water.obj");
        object_names.push_back("data/quaternius/Street_Bridge_WaterRamp.obj");
        object_names.push_back("data/quaternius/Street_Curve.obj");
        object_names.push_back("data/quaternius/Street_Deadend.obj");
        object_names.push_back("data/quaternius/Street_Elevated.obj");
        object_names.push_back("data/quaternius/Street_Elevated_Ramp.obj");
        object_names.push_back("data/quaternius/Street_Empty.obj");
        object_names.push_back("data/quaternius/Street_Empty_Water.obj");
        object_names.push_back("data/quaternius/Street_Straight.obj");

        for (const char* c : object_names)
            m_objets.push_back(read_mesh(c));
        
        m_objets[0].bounds(pmin, pmax);
        for (Mesh m : m_objets) {
            Point local_pmin, local_pmax;
            m.bounds(local_pmin, local_pmax);
            pmin = min(pmin, local_pmin);
            pmax = max(pmax, local_pmax);
        }
        m_camera.lookat(distance_factor * nbr_rows * pmin, distance_factor * nbr_rows * pmax);

        m_program = read_program(m_program_name);
        program_print_errors(m_program);
        glUseProgram(m_program);

        nbr_object = object_names.size();
        vertex_buffer = new GLuint[nbr_object];
        glGenBuffers(nbr_object, vertex_buffer);
        for (uint i = 0; i < nbr_object; ++i) {
            glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer[i]);
            glBufferData(GL_ARRAY_BUFFER,
                        m_objets[i].vertex_buffer_size() + m_objets[i].normal_buffer_size(),
                        nullptr, GL_STATIC_DRAW);
            glBufferSubData(GL_ARRAY_BUFFER, 0,
                            m_objets[i].vertex_buffer_size(), m_objets[i].vertex_buffer());
            glBufferSubData(GL_ARRAY_BUFFER, m_objets[i].vertex_buffer_size(),
                            m_objets[i].normal_buffer_size(), m_objets[i].normal_buffer());
            glBindBuffer(GL_ARRAY_BUFFER, 0);
        }
        
        vao = new GLuint[nbr_object];
        glGenVertexArrays(nbr_object, vao);
        for (uint i = 0; i < nbr_object; ++i) {
            glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer[i]);
            glBindVertexArray(vao[i]);

            GLint position_attribute = glGetAttribLocation(m_program, "position");
            glVertexAttribPointer(position_attribute, 3, GL_FLOAT, GL_FALSE, 0, 0);
            glEnableVertexAttribArray(position_attribute);

            GLint normal_attribute = glGetAttribLocation(m_program, "normal");
            glVertexAttribPointer(normal_attribute, 3, GL_FLOAT, GL_FALSE,
                0, (const void*) m_objets[i].vertex_buffer_size());
            glEnableVertexAttribArray(normal_attribute);
        }

        glUseProgram(0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);

        // etat openGL par defaut
        glClearColor(0.2f, 0.2f, 0.2f, 1.f);        // couleur par defaut de la fenetre
        
        glClearDepth(1.f);                          // profondeur par defaut
        glDepthFunc(GL_LESS);                       // ztest, conserver l'intersection la plus proche de la camera
        glEnable(GL_DEPTH_TEST);                    // activer le ztest

        return 0;   // ras, pas d'erreur
    }
    
    // destruction des objets de l'application
    int quit( )
    {
        release_program(m_program);
        for (uint i = 0; i < nbr_object; ++i)
            m_objets[i].release();
        glDeleteBuffers(nbr_object, vertex_buffer);
        glDeleteVertexArrays(nbr_object, vao);
        free(vertex_buffer);
        
        return 0;
    }
    
    // dessiner une nouvelle image
    int render( )
    {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // deplace la camera
        int mx, my;
        unsigned int mb= SDL_GetRelativeMouseState(&mx, &my);
        if(mb & SDL_BUTTON(1))              // le bouton gauche est enfonce
            m_camera.rotation(mx, my);
        else if(mb & SDL_BUTTON(3))         // le bouton droit est enfonce
            m_camera.move(mx);
        else if(mb & SDL_BUTTON(2))         // le bouton du milieu est enfonce
            m_camera.translation((float) mx / (float) window_width(), (float) my / (float) window_height());

        // recharge le shader
        if(key_state('r'))
        {
            clear_key_state('r');
            reload_program(m_program, m_program_name);
            program_print_errors(m_program);
        } 

        // rendu

        glUseProgram(m_program);

        Transform view = m_camera.view();
        Transform projection = m_camera.projection(window_width(), window_height(), 45);
        program_uniform(m_program, "viewInvMatrix", view.inverse());
        program_uniform(m_program, "pos_light", pmax);
        program_uniform(m_program, "light_color", vec3(1.0f, 1.0f, 1.0f));

        int mesh_nbr = 0;
        for (int trans_x = -nbr_rows + 1; trans_x <= nbr_rows - 1; trans_x++)
        {
            for (int trans_z = -nbr_rows + 1; trans_z <= nbr_rows - 1; trans_z++)
            {
                glBindVertexArray(vao[mesh_nbr]);
                Transform translate = 
                    Translation(Vector(pmax, pmin).x * distance_factor * trans_x,
                                0,
                                Vector(pmax, pmin).z * distance_factor * trans_z);

                Transform model = translate * RotationY(global_time() / SPEED_FACTOR);
                Transform mvp = projection * view * model;
                program_uniform(m_program, "mvpMatrix", mvp);
                program_uniform(m_program, "modelMatrix", model);
                program_uniform(m_program, "indexMesh", mesh_nbr);
                glDrawArrays(GL_TRIANGLES, 0, m_objets[mesh_nbr].vertex_count());
                mesh_nbr++;
                mesh_nbr = mesh_nbr % m_objets.size();
            }
        }

        glBindVertexArray(0);
        glUseProgram(0);

        return 1;
    }

protected:
    const float SPEED_FACTOR = 6000; // SPEED_FACTOR

    std::vector<Mesh> m_objets;
    Orbiter m_camera;
    Point pmin, pmax;
    
    int nbr_rows = 5;
    float distance_factor = 1.5;

    const char* m_program_name = "tps/shaders/shader_2.glsl";
    GLuint m_program;
    uint nbr_object;
    GLuint *vertex_buffer;
    GLuint *vao;
};


int main( int argc, char **argv )
{
    TP tp;
    tp.run();
    
    return 0;
}
