#include "Mesh.h"

#include <sstream>
#include <fstream>

std::istream& operator>>(std::istream& is, char const* s)
{
    if (s == nullptr)
        return is;

    if (is.flags() & std::ios::skipws) {
        while (std::isspace(is.peek()))
            is.ignore(1);

        while (std::isspace((unsigned char)*s))
            ++s;
    }

    while (*s && is.peek() == *s) {
        is.ignore(1);
        ++s;
    }
    if (*s)
        is.setstate(std::ios::failbit);
    return is;
}

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
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (const void*)offset);
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

    std::ifstream file;
    file.open(filename);

    if (!file.is_open()) {
        std::cout << "Error : Couldn't open Mesh file" << std::endl;
        return Mesh();
    }

    std::vector<glm::vec3> positions_tmp;
    std::vector<glm::vec2> texcoords_tmp;
    std::vector<glm::vec3> normals_tmp;

    std::string line;
    while (getline(file, line))
    {
        std::stringstream ss(line);
        std::string tag;

        // istream& operator>> skips whitespaces unless std::skipws is disabled
        // ignore empty lines
        if (!(ss >> tag))
            continue;

        // ignore comments
        if (tag[0] == '#')
            continue;

        float x, y, z;
        if (tag == "v")
        {
            if (!(ss >> x >> y >> z))
                break;
            positions_tmp.emplace_back(x, y, z);
        }
        else if (tag == "vt")
        {
            if (!(ss >> x >> y))
                break;
            texcoords_tmp.emplace_back(x, y);
        }
        else if (tag == "vn")
        {
            if (!(ss >> x >> y >> z))
                break;
            normals_tmp.emplace_back(x, y, z);
        }
        else if (tag == "f")
        {
            for (int i = 0; i < 3; ++i)
            {
                unsigned int idp(0), idt(0), idn(0);

                // reset stringstream after each read so that it keeps current position when fail
                auto pos = ss.tellg();
                auto state = ss.rdstate() && ~std::ios_base::failbit;
                auto reset = [state, pos](std::stringstream& ss)
                { ss.clear(state); ss.seekg(pos); return !ss.fail(); };

                // will try to match either i//i or i/i/i or i/i or i (all .obj "f" configurations)
                if (reset(ss) && ss >> idp >> "//" >> idn) {}
                else if (reset(ss) && ss >> idp >> "/" >> idt >> "/" >> idn) {}
                else if (reset(ss) && ss >> idp >> "/" >> idt) {}
                else if (reset(ss) && ss >> idp) {}

                // do stuff with indexes
                // ...
                mesh.indices.push_back(idp -1);
            }
        }
    }
    mesh.positions = positions_tmp;

    // happens when quit reading loop from sscanf fail
    if (!file.eof())
        std::cout << "[error] loading mesh " << filename << "..." << line << "\n" << std::endl;

#if 0
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
#endif

    file.close();

    return mesh;
}
