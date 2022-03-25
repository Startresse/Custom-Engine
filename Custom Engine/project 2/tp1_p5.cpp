#include "app_time.h"
#include "draw.h"        
#include "wavefront.h"

class TP : public AppTime
{
public:
    TP() : AppTime(1024, 640) {}
    TP(uint x, uint y) : AppTime(x, y) {}
    
    int init( )
    {
        // load objects
        for (const char* c : object_names)
            m_objets.push_back(read_mesh(c));
        
        // set camera
        Point pmin, pmax;
        m_objets[0].bounds(pmin, pmax);
        for (Mesh m : m_objets) {
            Point local_pmin, local_pmax;
            m.bounds(local_pmin, local_pmax);
            pmin = min(pmin, local_pmin);
            pmax = max(pmax, local_pmax);
        }
        m_camera.lookat(pmin, pmax);

        // set fp
        glGenFramebuffers(1, &fbo);
        glBindFramebuffer(GL_FRAMEBUFFER, fbo);

        glGenTextures(1, &fp_tex);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, fp_tex);
        glTexImage2D(GL_TEXTURE_2D, 
            /* level */ 0,
            /* texel format */ GL_RGB32F, 
            /* width, height, border */ window_width(), window_height(), 0,
            /* data format */ GL_RGB32F, /* data type */ GL_FLOAT,
            /* data */ NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);

        glFramebufferTexture(GL_FRAMEBUFFER,
            /* attachment */ GL_COLOR_ATTACHMENT0,
            /* texture */ fp_tex,
            /* mipmap level */ 0);

        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        GLenum fbo_buffers[] = { GL_COLOR_ATTACHMENT0 };
        glDrawBuffers(1, fbo_buffers);

        // load shaders
        m_program_fp = read_program(fp_shader_name);
        program_print_errors(m_program_fp);

        m_program = read_program(m_program_name);
        program_print_errors(m_program);
        glUseProgram(m_program);

        // set buffers and vao for each object
        buffer = new GLuint[nbr_object];
        glGenBuffers(nbr_object, buffer);

        vao = new GLuint[nbr_object];
        glGenVertexArrays(nbr_object, vao);

        for (uint i = 0; i < nbr_object; ++i) {
            glBindBuffer(GL_ARRAY_BUFFER, buffer[i]);
            glBindVertexArray(vao[i]);

            Mesh& obj = m_objets[i];
            GLint location;
            size_t size = obj.vertex_buffer_size() +
                          obj.normal_buffer_size() + 
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

            location = glGetAttribLocation(m_program, "normal");
            glBufferSubData(GL_ARRAY_BUFFER, offset,
                obj.normal_buffer_size(), obj.normal_buffer());
            glVertexAttribPointer(location, 3, GL_FLOAT, GL_FALSE,
                0, (const void*) offset);
            glEnableVertexAttribArray(location);
            offset += obj.normal_buffer_size();

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

        glUseProgram(0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);

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
        release_program(m_program_fp);
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


        // setup
        Transform view = m_camera.view();
        Point camera = view.inverse()(Point(0, 0, 0));
        Transform projection = m_camera.projection(window_width(),
            window_height(), 45);
        GLint location;

        static uint frame = 0;
        if(key_state(SDLK_UP)) {
            clear_key_state(SDLK_UP);
            ++frame;
            frame %= nbr_object;
        }
        Mesh& object = m_objets[frame];
        glBindVertexArray(vao[frame]);

        Transform model = Identity();
        Transform mvp = projection * view * model;

        // first pass
        glBindFramebuffer(GL_FRAMEBUFFER, fbo);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        // glClearBuffer()
        glUseProgram(m_program_fp);

        location = glGetUniformLocation(m_program_fp, "mvpMatrix");
        glUniformMatrix4fv(location, 1, GL_TRUE, mvp.buffer());

        // rendu
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glUseProgram(m_program);

        location = glGetUniformLocation(m_program, "mvpMatrix");
        glUniformMatrix4fv(location, 1, GL_TRUE, mvp.buffer());
        location = glGetUniformLocation(m_program, "modelMatrix");
        glUniformMatrix4fv(location, 1, GL_TRUE, model.buffer());

        location = glGetUniformLocation(m_program, "emission");
        glUniform3f(location, 1.0f, 1.0f, 1.0f);
        location = glGetUniformLocation(m_program, "camera");
        glUniform3f(location, camera.x, camera.y, camera.z);
        location = glGetUniformLocation(m_program, "source");
        glUniform3f(location, camera.x, camera.y, camera.z);

#define NBR_MATERIALS 16

        std::vector<Color> color_diff;
        std::vector<Color> color_spec;
        std::vector<float> spec_coeff;
        color_diff.resize(NBR_MATERIALS);
        color_spec.resize(NBR_MATERIALS);
        spec_coeff.resize(NBR_MATERIALS);
        const Materials& materials = object.materials();
        assert(materials.count() <= int(color_diff.size()));

        for(int i= 0; i < materials.count(); i++) {
            color_diff[i] = materials.material(i).diffuse;
            color_spec[i] = materials.material(i).specular;
            spec_coeff[i] = materials.material(i).ns;
        }

        location = glGetUniformLocation(m_program, "color_diff");
        glUniform4fv(location, color_diff.size(), &color_diff[0].r);
        location = glGetUniformLocation(m_program, "color_spec");
        glUniform4fv(location, color_spec.size(), &color_spec[0].r);
        location = glGetUniformLocation(m_program, "spec_coeff");
        glUniform1fv(location, spec_coeff.size(), &spec_coeff[0]);

        glDrawArrays(GL_TRIANGLES, 0, object.vertex_count());

        glBindVertexArray(0);
        glUseProgram(0);

        return 1;
    }

protected:
    std::vector<Mesh> m_objets;
    Orbiter m_camera;
    
    const char* m_program_name = "tps/shaders/shader_5.glsl";
    GLuint m_program;
    GLuint *buffer;
    GLuint *vao;

    // rendu multipass
    GLuint fbo;
    GLuint fp_tex;
    GLuint m_program_fp;
    const char* fp_shader_name = "tps/shaders/shader_5_fp.glsl";

    std::vector<const char*> object_names = {
        "data/quaternius/robot/Robot.obj",
    };
    uint nbr_object = object_names.size();
};


int main( int argc, char **argv )
{
    TP tp;
    tp.run();
    
    return 0;
}
