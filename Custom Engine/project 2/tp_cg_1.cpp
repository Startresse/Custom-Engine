#include <vector>

#include "window.h"

#include "program.h"
#include "uniforms.h"



int main( int argc, char **argv )
{
    Window window= create_window(1024, 640);
    if(window == NULL)
        return 1;

    Context context= create_context(window, 4,3);
    if(context == NULL)
        return 1;

    GLuint program = 0;
    program = read_program("tps/shaders/compute_1.glsl");

    program_print_errors(program);


    /* CODE */
    int N = 1024;

    GLuint res_buffer = 0;
    std::vector<uint> data(N, 0);
    glGenBuffers(1, &res_buffer);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, res_buffer);
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(unsigned int) * N, data.data(), GL_STREAM_READ);

    glUseProgram(program);
    program_uniform(program, "N", (unsigned int) N);

    GLint threads[3]= { };
    glGetProgramiv(program, GL_COMPUTE_WORK_GROUP_SIZE, threads);
    int groups = N / threads[0] + (N % threads[0] ? 1 : 0);
    glDispatchCompute(groups, 1, 1);

    std::vector<uint> res(N, 0);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, res_buffer);
    glGetBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, sizeof(unsigned int) * res.size(), res.data());

    std::cout << "[";
    for (uint x : res)
        std::cout << x << ", ";
    std::cout << "\b\b]\b" << std::endl;

    /* ENDCODE */



    release_program(program);
    release_context(context);
    release_window(window);
    return 0;
}

