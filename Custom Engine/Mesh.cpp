#include "Mesh.h"

#include <sstream>
#include <fstream>

unsigned int Mesh::create_buffers()
{
    if (positions.size() == 0)
        return 0;

    if (vao)
        return vao;

    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    size_t index_buffer_size;
    index_buffer_size = sizeof(unsigned int) * indices.size();
    glGenBuffers(1, &index_buffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, index_buffer);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, index_buffer_size, indices.data(), GL_STATIC_DRAW);

    size_t buffer_size;
    buffer_size = sizeof(glm::vec3) * positions.size();
    // TODO update depending on normals, etc...

    glGenBuffers(1, &buffer);
    glBindBuffer(GL_ARRAY_BUFFER, buffer);
    glBufferData(GL_ARRAY_BUFFER, buffer_size, nullptr, GL_STATIC_DRAW);

    // TODO add other stuff
    size_t offset = 0;
    size_t size = sizeof(glm::vec3) * positions.size();
    glBufferSubData(GL_ARRAY_BUFFER, offset, size, positions.data());
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (const void *) offset);
    glEnableVertexAttribArray(0);

    //offset += size;
    //size = normals.size();
    //glBufferSubData(GL_ARRAY_BUFFER, offset, size, normals.data());
    //glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, (const void *) offset);
    //glEnableVertexAttribArray(1);


    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    return vao;
}

void Mesh::draw(GLuint program)
{
    if (!vao)
        create_buffers();

    assert(vao);

    if (!program)
    {
        std::cout << "[error] (can't draw) : no program" << std::endl;
        return;
    }

    glUseProgram(program);

    glBindVertexArray(vao);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, index_buffer);
    glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(indices.size()), GL_UNSIGNED_INT, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    glUseProgram(0);
}

Mesh::~Mesh()
{
    if (vao)
        glDeleteVertexArrays(1, &vao);
    if (buffer)
        glDeleteBuffers(1, &buffer);
    if (index_buffer)
        glDeleteBuffers(1, &index_buffer);
}

Mesh read_mesh(std::string filename)
{
    Mesh mesh;

    mesh.positions = {
        glm::vec3( 0.5f,  0.5f, 0.0f),
        glm::vec3(-0.5f,  0.5f, 0.0f),
        glm::vec3(-0.5f, -0.5f, 0.0f),
        glm::vec3( 0.5f, -0.5f, 0.0f),
    };
    mesh.indices = { 0, 1, 2, 0, 2, 3 };

    std::ifstream file;
    file.open(filename);

    if (!file.is_open()) {
        std::cout << "Error : Couldn't open Mesh file" << std::endl;
        return Mesh();
    }

    std::vector<glm::vec3> positions_tmp;
    std::vector<glm::vec3> normals_tmp;
    std::vector<glm::vec2> texcoords_tmp;

    std::string line;
    while (getline(file, line))
    {
        // remove starting spaces
        size_t start = line.find_first_not_of(" \n\r\t\f\v");
        if (start == std::string::npos)
            line = "";
        else if (start != 0)
            line = line.substr(start);

        // ignore empty lines and commented lines
        if (line.empty() || line[0] == '#')
        {
            if (line.empty())
                std::cout << "EMPTY LINE" << std::endl;
            else
                std::cout << "COMMENTED : " << line << std::endl;
            continue;
        }

        std::cout << line << std::endl;

        std::stringstream ss(line);
        std::string word;

        if (line[0] == 'v')
        {
            float x, y, z;
            if (line[1] == ' ')
            {
                if(sscanf_s(line.c_str(), "v %f %f %f", &x, &y, &z) != 3)
                    break;
                positions_tmp.emplace_back(x, y, z);
            }
            if (line[1] == 'n')
            {
                if(sscanf_s(line.c_str(), "vn %f %f %f", &x, &y, &z) != 3)
                    break;
                normals_tmp.emplace_back(x, y, z);
            }
            if (line[1] == 't')
            {
                if(sscanf_s(line.c_str(), "vt %f %f", &x, &y) != 2)
                    break;
                texcoords_tmp.emplace_back(x, y);
            }
        }
        else if (line[0] == 'f')
        {

        }

    }

    // happens when quit reading loop from sscanf fail
    if (!file.eof())
        std::cout << "[error] loading mesh " << filename << "..." << line << "\n" << std::endl;

    for (auto x : positions_tmp)
        std::cout << "(" << x.x << ", " << x.y << ", " << x.z << ")" << std::endl;
    std::cout << std::endl;
    for (auto x : normals_tmp)
        std::cout << "(" << x.x << ", " << x.y << ", " << x.z << ")" << std::endl;
    std::cout << std::endl;
    for (auto x : texcoords_tmp)
        std::cout << "(" << x.x << ", " << x.y << ")" << std::endl;
    std::cout << std::endl;
    mesh.positions = positions_tmp;

    file.close();

    exit(0);

    return mesh;
}
