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

    //mesh.positions = {
    //    glm::vec3(0.5f,  0.5f, 0.0f),
    //    glm::vec3(-0.5f,  0.5f, 0.0f),
    //    glm::vec3(-0.5f, -0.5f, 0.0f),
    //    glm::vec3(0.5f, -0.5f, 0.0f),
    //};
    //mesh.indices = { 0, 1, 2, 0, 2, 3 };

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
        if (!(ss >> tag))
        {
            std::cout << "EMPTY LINE" << std::endl;
            continue;
        }

        if (tag[0] == '#')
        {
            std::cout << "COMMENTED : " << line << std::endl;
            continue;
        }

        std::cout << line << std::endl;

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
            std::vector<unsigned int> idp;
            std::vector<unsigned int> idt;
            std::vector<unsigned int> idn;
            for (int i = 0; i < 3; ++i)
            {
                idp.push_back(0);
                idt.push_back(0);
                idn.push_back(0);

                // save current stringstream position
                auto state = ss.rdstate() && ~std::ios_base::failbit;
                auto pos = ss.tellg();

                auto reset = [state, pos](std::stringstream& ss)
                { ss.clear(state); ss.seekg(pos); return !ss.fail(); };

                if (reset(ss) && ss >> idp.back() >> "//" >> idn.back())
                    continue;
                else if (reset(ss) && ss >> idp.back() >> "/" >> idt.back() >> "/" >> idn.back())
                    continue;
                else if (reset(ss) && ss >> idp.back() >> "/" >> idt.back())
                    continue;
                else if (reset(ss) && ss >> idp.back() >> "/" >> idt.back())
                    continue;
                else
                    break;
            }
            for (int i = 0 ; i < 3; ++i)
                std::cout << idp[i] << ", " << idt[i] << ", " << idn[i] << std::endl;
        }
        std::cout << std::endl;

    }

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

    exit(0);

    return mesh;
}
