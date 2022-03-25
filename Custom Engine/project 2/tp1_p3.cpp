#include "app_time.h"
#include "draw.h"        
#include "wavefront.h"
#include "algorithm"


class TP : public AppTime
{
public:
    TP() : AppTime(1024, 640) {}
    TP(uint x, uint y) : AppTime(x, y) {}
    
    int init( )
    {
        /* OBJECT LOADING */
        for (const char* c : object_names)
            m_objets.push_back(read_mesh(c));

        
        /* CAMERA AND LOADOUT SETUP */
        Point pmin, pmax;
        m_objets[0].bounds(pmin, pmax);
        for (Mesh m : m_objets) {
            Point local_pmin, local_pmax;
            m.bounds(local_pmin, local_pmax);
            pmin = min(pmin, local_pmin);
            pmax = max(pmax, local_pmax);
        }
        std::vector<float> values = {abs(pmax.x), abs(pmax.y), abs(pmax.z),
                                     abs(pmin.x), abs(pmin.y), abs(pmin.z)};
        distance_factor = *max_element(values.begin(), values.end());
    
        m_camera.lookat(Point(nbr_rows * pmin.x, pmin.y, nbr_rows * pmin.z),
                        Point(nbr_rows * pmax.x, pmax.y, nbr_rows * pmax.z));


        /* SHADER SETUP */
        m_program = read_program(m_program_name);
        program_print_errors(m_program);
        glUseProgram(m_program);


        /* BUFFER AND VAO SETUP */
        buffer = new GLuint[nbr_object];
        glGenBuffers(nbr_object, buffer);

        vao = new GLuint[nbr_object];
        glGenVertexArrays(nbr_object, vao);

        for (uint i = 0; i < nbr_object; ++i) {
            glBindBuffer(GL_ARRAY_BUFFER, buffer[i]);
            glBindVertexArray(vao[i]);

            Mesh& obj = m_objets[i];
            Mesh& obj_target = m_objets[(i + 1) % nbr_object];
            GLint location;
            size_t size = obj.vertex_buffer_size() + obj_target.vertex_buffer_size() +
                          obj.normal_buffer_size() + obj_target.normal_buffer_size() +
                          obj.triangle_count() * 3;
            glBufferData(GL_ARRAY_BUFFER, size, nullptr, GL_STATIC_DRAW);
            size_t offset = 0;

            location = glGetAttribLocation(m_program, "position");
            glBufferSubData(GL_ARRAY_BUFFER, offset,
                obj.vertex_buffer_size(), obj.vertex_buffer());
            glVertexAttribPointer(location, 3, GL_FLOAT, GL_FALSE,
                0, (const void*)offset);
            glEnableVertexAttribArray(location);
            offset += obj.vertex_buffer_size();

            location = glGetAttribLocation(m_program, "position_target");
            glBufferSubData(GL_ARRAY_BUFFER, offset,
                obj_target.vertex_buffer_size(), obj_target.vertex_buffer());
            glVertexAttribPointer(location, 3, GL_FLOAT, GL_FALSE,
                0, (const void*)offset);
            glEnableVertexAttribArray(location);
            offset += obj_target.vertex_buffer_size();

            location = glGetAttribLocation(m_program, "normal");
            glBufferSubData(GL_ARRAY_BUFFER, offset,
                obj.normal_buffer_size(), obj.normal_buffer());
            glVertexAttribPointer(location, 3, GL_FLOAT, GL_FALSE,
                0, (const void*) offset);
            glEnableVertexAttribArray(location);
            offset += obj.normal_buffer_size();

            location = glGetAttribLocation(m_program, "normal_target");
            glBufferSubData(GL_ARRAY_BUFFER, offset,
                obj_target.normal_buffer_size(), obj_target.normal_buffer());
            glVertexAttribPointer(location, 3, GL_FLOAT, GL_FALSE,
                0, (const void*) offset);
            glEnableVertexAttribArray(location);
            offset += obj_target.normal_buffer_size();

            size_t size_buffer = obj.triangle_count() * 3;
            std::vector<unsigned char> buffer(size_buffer);
            for(int i= 0; i < int(obj.triangle_count()); i++) {
                int index = obj.triangle_material_index(i);
                buffer[3*i] = index;
                buffer[3*i+1] = index;
                buffer[3*i+2] = index;
            }
            location = glGetAttribLocation(m_program, "material_index");
            glBufferSubData(GL_ARRAY_BUFFER, offset,
                size_buffer, buffer.data()); 
            glVertexAttribIPointer(location, 1, GL_UNSIGNED_BYTE,
                0, (const void*) offset);
            glEnableVertexAttribArray(location);
        }


        /* CLEAR BINDINGS */
        glUseProgram(0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);


        /* ARRAYS INITILIZATION */
        last_time_v = std::vector<float>(nbr_rows * nbr_rows, 0.0f);
        current_obj_v = std::vector<uint>(nbr_rows * nbr_rows, 0);
        time_between_frame_v = std::vector<float>(nbr_rows * nbr_rows);
        for (int i = 0; i < nbr_rows * nbr_rows; ++i)
            time_between_frame_v[i] = SPEED_FACTOR * 1000/(24.0f * (10.0f*rand()/RAND_MAX + 24.0f));

        // Materials
#define NBR_MATERIALS 16

        color_diff.resize(NBR_MATERIALS);
        color_spec.resize(NBR_MATERIALS);
        spec_coeff.resize(NBR_MATERIALS);
        const Materials& materials = m_objets[0].materials();
        assert(materials.count() <= int(color_diff.size()));

        for(int i= 0; i < materials.count(); i++) {
            color_diff[i] = materials.material(i).diffuse;
            color_spec[i] = materials.material(i).specular;
            spec_coeff[i] = materials.material(i).ns;
        }


        /* GL INIT */
        glClearColor(0.029f, 0.029f, 0.029f, 1.f);
        glClearDepth(1.f);
        glDepthFunc(GL_LESS);
        glEnable(GL_DEPTH_TEST);
        glEnable(GL_FRAMEBUFFER_SRGB);

        return 0;
    }
    
    int quit( )
    {
        release_program(m_program);

        for (uint i = 0; i < nbr_object; ++i)
            m_objets[i].release();

        glDeleteBuffers(nbr_object, buffer);
        glDeleteVertexArrays(nbr_object, vao);
        free(buffer);
        free(vao);
        
        return 0;
    }
    
    int render( )
    {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


        /* MOUSE HANDLING */
        int mx, my;
        unsigned int mb= SDL_GetRelativeMouseState(&mx, &my);
        if(mb & SDL_BUTTON(1))              // le bouton gauche est enfonce
            m_camera.rotation(mx, my);
        else if(mb & SDL_BUTTON(3))         // le bouton droit est enfonce
            m_camera.move(mx);
        else if(mb & SDL_BUTTON(2))         // le bouton du milieu est enfonce
            m_camera.translation((float) mx / (float) window_width(), (float) my / (float) window_height());


        /* SHADER RELOAD */
        if(key_state('r'))
        {
            clear_key_state('r');
            reload_program(m_program, m_program_name);
            program_print_errors(m_program);
        } 


        /* RENDER */
        glUseProgram(m_program);

        Transform view = m_camera.view();
        Point camera = view.inverse()(Point(0, 0, 0));
        Transform projection = m_camera.projection(window_width(),
            window_height(), 45);

        // lighting
        GLint location;
        location = glGetUniformLocation(m_program, "emission");
        glUniform3f(location, 1.0f, 1.0f, 1.0f);
        location = glGetUniformLocation(m_program, "camera");
        glUniform3f(location, camera.x, camera.y, camera.z);
        location = glGetUniformLocation(m_program, "source");
        glUniform3f(location, camera.x, camera.y, camera.z);

        // Materials
        location = glGetUniformLocation(m_program, "color_diff");
        glUniform4fv(location, color_diff.size(), &color_diff[0].r);
        location = glGetUniformLocation(m_program, "color_spec");
        glUniform4fv(location, color_spec.size(), &color_spec[0].r);
        location = glGetUniformLocation(m_program, "spec_coeff");
        glUniform1fv(location, spec_coeff.size(), &spec_coeff[0]);

        // Time for vertex LERP
        float current_time = global_time();

        for (int mesh_nbr = 0; mesh_nbr < nbr_rows * nbr_rows; ++mesh_nbr)
        {
            // Mesh related variables
            float& last_time = last_time_v[mesh_nbr];
            uint& current_obj = current_obj_v[mesh_nbr];
            float& time_between_frame = time_between_frame_v[mesh_nbr];

            // "Frame" handling
            if (current_time > last_time + time_between_frame) {
                last_time = current_time;
                current_obj++;
                current_obj %= nbr_object;
            }
            Mesh& object = m_objets[current_obj];
            glBindVertexArray(vao[current_obj]);

            location = glGetUniformLocation(m_program, "t");
            glUniform1f(location, (current_time - last_time)/time_between_frame);

            // Space positionning
            float x = distance_factor * (mesh_nbr % nbr_rows - nbr_rows / 2);
            float z = distance_factor * (mesh_nbr / nbr_rows - nbr_rows / 2);
            Transform model = Translation(x, 0, z);
            Transform mvp = projection * view * model;
            location = glGetUniformLocation(m_program, "mvpMatrix");
            glUniformMatrix4fv(location, 1, GL_TRUE, mvp.buffer());
            location = glGetUniformLocation(m_program, "modelMatrix");
            glUniformMatrix4fv(location, 1, GL_TRUE, model.buffer());

            // Draw call
            glDrawArrays(GL_TRIANGLES, 0, object.vertex_count());
        }


        /* GL CLEANUP */
        glBindVertexArray(0);
        glUseProgram(0);

        return 1;
    }

protected:
    const float SPEED_FACTOR = 100;

    Point pmin, pmax;

    std::vector<Mesh> m_objets;
    Orbiter m_camera;
    
    const char* m_program_name = "tps/shaders/shader_3.glsl";
    GLuint m_program;
    GLuint *buffer;
    GLuint *vao;

    std::vector<float> last_time_v;
    std::vector<uint> current_obj_v;
    std::vector<float> time_between_frame_v;

    int nbr_rows = 21;
    float distance_factor;

    std::vector<Color> color_diff;
    std::vector<Color> color_spec;
    std::vector<float> spec_coeff;


    std::vector<const char*> object_names = {
        "data/quaternius/robot/run/Robot_000001.obj",
        "data/quaternius/robot/run/Robot_000002.obj",
        "data/quaternius/robot/run/Robot_000003.obj",
        "data/quaternius/robot/run/Robot_000004.obj",
        "data/quaternius/robot/run/Robot_000005.obj",
        "data/quaternius/robot/run/Robot_000006.obj",
        "data/quaternius/robot/run/Robot_000007.obj",
        "data/quaternius/robot/run/Robot_000008.obj",
        "data/quaternius/robot/run/Robot_000009.obj",
        "data/quaternius/robot/run/Robot_000010.obj",
        "data/quaternius/robot/run/Robot_000011.obj",
        "data/quaternius/robot/run/Robot_000012.obj",
        "data/quaternius/robot/run/Robot_000013.obj",
        "data/quaternius/robot/run/Robot_000014.obj",
        "data/quaternius/robot/run/Robot_000015.obj",
        "data/quaternius/robot/run/Robot_000016.obj",
        "data/quaternius/robot/run/Robot_000017.obj",
        "data/quaternius/robot/run/Robot_000018.obj",
        "data/quaternius/robot/run/Robot_000019.obj",
        "data/quaternius/robot/run/Robot_000020.obj",
        "data/quaternius/robot/run/Robot_000021.obj",
        "data/quaternius/robot/run/Robot_000022.obj",
        "data/quaternius/robot/run/Robot_000023.obj",
    };
    uint nbr_object = object_names.size();
};


int main( int argc, char **argv )
{
    TP tp;
    tp.run();
    
    return 0;
}
