#include "tp3_p2.h"


/* BVH */

int BVH::build(const Mesh &mesh)
{
    std::vector<Triangle> trs;
    for (int i = 0; i < mesh.triangle_count(); ++i)
    {
        const TriangleData &trd = mesh.triangle(i);
        trs.emplace_back(trd.a, trd.b, trd.c, mesh.triangle_material_index(i), i);
    }

    BBox box;
    mesh.bounds(box.pmin, box.pmax);

    int root = build(box, trs);
    int nb_src = build_sources(mesh);
    std::cout << "Scene contains " << nb_src << " light sources" << std::endl;

    return root;
}

int BVH::build(const BBox &_bounds, const std::vector<Triangle> &_triangles)
{
    triangles = _triangles; // copie les triangles pour les trier
    nodes.clear();          // efface les noeuds
    nodes.reserve(triangles.size());

    root = build(_bounds, 0, triangles.size());
    sew();

    return root;
}

int BVH::build(const BBox &bounds, const int begin, const int end)
{
    if (end - begin < 2)
    {
        // inserer une feuille et renvoyer son indice
        int index = nodes.size();
        nodes.push_back(make_leaf(bounds, begin, end));
        return index;
    }

    // axe le plus etire de l'englobant
    Vector d = Vector(bounds.pmin, bounds.pmax);
    int axis;
    if (d.x > d.y && d.x > d.z) // x plus grand que y et z ?
        axis = 0;
    else if (d.y > d.z) // y plus grand que z ? (et que x implicitement)
        axis = 1;
    else // x et y ne sont pas les plus grands...
        axis = 2;

    // coupe l'englobant au milieu
    float cut = bounds.centroid(axis);

    // repartit les triangles
    Triangle *pm = std::partition(triangles.data() + begin, triangles.data() + end, triangle_less1(axis, cut));
    int m = std::distance(triangles.data(), pm);

    // la repartition des triangles peut echouer, et tous les triangles sont dans la meme partie...
    // forcer quand meme un decoupage en 2 ensembles
    if (m == begin || m == end)
        m = (begin + end) / 2;
    assert(m != begin);
    assert(m != end);

    // construire le fils gauche
    // les triangles se trouvent dans [begin .. m)
    BBox bounds_left = triangle_bounds(begin, m);
    int left = build(bounds_left, begin, m);

    // on recommence pour le fils droit
    // les triangles se trouvent dans [m .. end)
    BBox bounds_right = triangle_bounds(m, end);
    int right = build(bounds_right, m, end);

    int index = nodes.size();
    nodes.push_back(make_node(bounds, left, right));
    return index;
}

BBox BVH::triangle_bounds(const int begin, const int end)
{
    BBox bbox = triangles[begin].bounds();
    for (int i = begin + 1; i < end; i++)
        bbox.insert(triangles[i].bounds());

    return bbox;
}

std::vector<SewedNode> BVH::sew(int node, int skip)
{
    const Node& n = nodes[node];
    SewedNode sn;
    std::vector<SewedNode> ret;
    if (!n.leaf())
    {
        sn = SewedNode(n.bounds, n.left, skip, -1);
        const std::vector<SewedNode>& left_vect = sew(n.left, n.right);
        const std::vector<SewedNode>& right_vect = sew(n.right, skip);
        ret.insert(ret.end(), left_vect.begin(), left_vect.end());
        ret.insert(ret.end(), right_vect.begin(), right_vect.end());
    }
    else
    {
        assert(n.left - n.right == 1);
        sn = SewedNode(n.bounds, skip, skip, -n.left);
    }
    ret.push_back(sn);
    return ret;
}

int BVH::build_sources(const Mesh& mesh)
{
    for (uint i = 0; i < triangles.size(); ++i)
    {
        if (mesh.triangle_material(triangles[i].id).emission.power() > 0)
            sources.push_back(i);
    }

    return sources.size();
}

/* APP */

int RT::init()
{
    if (m_mesh.triangle_count() == 0)
        return -1;

    /* CAMERA */

    Point pmin, pmax;
    m_mesh.bounds(pmin, pmax);
    m_camera.lookat(pmin, pmax);


    /* INIT BUFFER AND TEXTURES */

    init_buffers();
    init_textures();


    /* SHADERS */

    for (auto shader : shaders)
    {
        *(shader.first) = read_program(shader.second);
        program_print_errors(*(shader.first));
    }


    /* BINDING */

    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, triangle_buffer);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, material_buffer);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, sources_buffer);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, nodes_buffer);

    glBindImageTexture(0, resultat_tex,  0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA16);
    glBindImageTexture(1, seeds_tex,     0, GL_FALSE, 0, GL_READ_WRITE, GL_R32UI);
    glBindImageTexture(2, ray_tex,       0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F);
    glBindImageTexture(3, color_tex,     0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA16);


    /* GL CLEANING */

    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
    glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
    glBindTexture(GL_TEXTURE_2D, 0);

    return 0;
}

void RT::init_buffers()
{
    for (auto buffer : buffers)
        glGenBuffers(buffer.second, buffer.first);

    /* TRIANGLES */
    BVH bvh;
    root = bvh.build(m_mesh);
    program_uniform(intersect_shader, "root", root);
    program_uniform(ambiant_shader,   "root", root);
    program_uniform(light_shader,     "root", root);
    program_uniform(L2_shader,        "root", root);


    // All triangles
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, triangle_buffer);
    glBufferData(GL_SHADER_STORAGE_BUFFER,
                    bvh.triangles.size() * sizeof(Triangle), bvh.triangles.data(), GL_STATIC_READ);

    // Light source triangles
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, sources_buffer);
    glBufferData(GL_SHADER_STORAGE_BUFFER,
                    bvh.sources.size() * sizeof(int), bvh.sources.data(), GL_STATIC_READ);

    // BVH sewed nodes
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, nodes_buffer);
    glBufferData(GL_SHADER_STORAGE_BUFFER,
                    bvh.sewed_nodes.size() * sizeof(SewedNode), bvh.sewed_nodes.data(), GL_STATIC_READ);


    /* MATERIALS */
    std::vector<Material> big_material_data = m_mesh.materials().materials;
    std::vector<smallMat> material_data;
    material_data.reserve(big_material_data.size());
    for (size_t i = 0; i < big_material_data.size(); ++i)
    {
        const Material &mat = big_material_data[i];

        smallMat sm;
        sm.emission = mat.emission;
        sm.diffuse = mat.diffuse;
        sm.specular = mat.specular;
        sm.specular.a = mat.ns;

        material_data.push_back(sm);
    }

    glBindBuffer(GL_SHADER_STORAGE_BUFFER, material_buffer);
    glBufferData(GL_SHADER_STORAGE_BUFFER,
                    material_data.size() * sizeof(smallMat), material_data.data(), GL_STATIC_READ);

}

void RT::init_textures()
{
    // TEXTURE GENERATION
    for (auto texture : textures)
        glGenTextures(texture.second, texture.first);

    // RANDOM_SEEDS
    std::random_device rng;
    std::vector<uint> data_seeds(window_height() * window_width());
    for (int i = 0; i < window_width() * window_height(); ++i)
        data_seeds[i] = rng();

    glBindTexture(GL_TEXTURE_2D, seeds_tex);
    glTexImage2D(GL_TEXTURE_2D, 0,
                    GL_R32UI, window_width(), window_height(), 0,
                    GL_RED_INTEGER, GL_UNSIGNED_INT, data_seeds.data());
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);

    // RAY IMAGE
    glBindTexture(GL_TEXTURE_2D_ARRAY, ray_tex);
    glTexStorage3D(GL_TEXTURE_2D_ARRAY, 1, GL_RGBA32F,
                   window_width(), window_height(), 3);

    // CURRENT IMAGE COLOR
    // 0 : L0
    // 1 : L1
    // 2 : L2
    // 3 : AO
    glBindTexture(GL_TEXTURE_2D_ARRAY, color_tex);
    glTexStorage3D(GL_TEXTURE_2D_ARRAY, 1, GL_RGBA16,
                   window_width(), window_height(), 4);

    // FINAL IMAGE
    // 0 : L0
    // 1 : L1
    // 2 : L2
    // 3 : AO
    // 4 : Resulting image
    glBindTexture(GL_TEXTURE_2D_ARRAY, resultat_tex);
    glTexStorage3D(GL_TEXTURE_2D_ARRAY, 1, GL_RGBA16,
                   window_width(), window_height(), 5);

    // BLIT FRAMEBUFFER
    glGenFramebuffers(1, &blit_framebuffer);
    glBindFramebuffer(GL_READ_FRAMEBUFFER, blit_framebuffer);
    glFramebufferTextureLayer(GL_READ_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, resultat_tex, 0, 4);
    glReadBuffer(GL_COLOR_ATTACHMENT0);
}

int RT::quit()
{
    for (auto shader : shaders)
        release_program(*(shader.first));

    for (auto texture : textures)
        glDeleteTextures(texture.second, texture.first);

    for (auto buffer : buffers)
        glDeleteBuffers(buffer.second, buffer.first);

    glDeleteFramebuffers(1, &blit_framebuffer);

    return 0;
}

int RT::render()
{
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
    glViewport(0, 0, window_width(), window_height());
    glClear(GL_COLOR_BUFFER_BIT);


    /* INPUT */

    bool reset_frame = handle_input();
    if (reset_frame)
        frame = 0;
    else
        frame++;


    /* RENDER */

    m = Identity();
    v = m_camera.view();
    p = m_camera.projection(window_width(), window_height(), 45);
    im = m_camera.viewport();
    T = im * p * v * m;
    T_inv = T.inverse();

    int nx = window_width() / 8;
    int ny = window_height() / 8;

    if (frame < nb_max_frame)
    {
        // Reset frame color
        float color[4]= { 0, 0, 0, 0 };
        glClearTexImage(color_tex, 0, GL_RGBA, GL_FLOAT, color);

        /* INTERSECTION */
        glUseProgram(intersect_shader);
        program_uniform(intersect_shader, "invMatrix", T_inv);
        glDispatchCompute(nx, ny, 1);

        glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

        /* AMBIANT */
        if (ambiant_strength > 0.0)
        {
            glUseProgram(ambiant_shader);
            program_uniform(ambiant_shader, "ambiant_strength", ambiant_strength);
            glDispatchCompute(nx, ny, 1);
        }

        /* LIGHT */
        if (toggle_Ldirect)
        {
            // L0 & L1
            glUseProgram(light_shader);
            glDispatchCompute(nx, ny, 1);
        }

        if (toggle_Lindirect)
        {
            // L2
            glUseProgram(L2_shader);
            glDispatchCompute(nx, ny, 1);
        }

        glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

        /* ACCUMULATION */
        glUseProgram(accu_shader);
        program_uniform(accu_shader, "frame", frame);
        glDispatchCompute(nx, ny, 1);
    }


    /* BLIT BUFFER */

    glMemoryBarrier(GL_TEXTURE_UPDATE_BARRIER_BIT);
    glBindFramebuffer(GL_READ_FRAMEBUFFER, blit_framebuffer);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
    glBlitFramebuffer(
        0, 0, window_width(), window_height(),
        0, 0, window_width(), window_height(),
        GL_COLOR_BUFFER_BIT, GL_NEAREST);

    return 1;
}

bool RT::handle_input()
{
    bool reset_frame = false;

    /* SHADER RELOAD */
    if (key_state('r'))
    {
        clear_key_state('r');
        for (auto shader : shaders)
        {
            reload_program(*(shader.first), shader.second);
            program_print_errors(*(shader.first));
        }

        reset_frame = true;
    }

    /* CAMERA RESET */
    if (key_state('f'))
    {
        clear_key_state('f');
        Point pmin, pmax;
        m_mesh.bounds(pmin, pmax);
        m_camera.lookat(pmin, pmax);

        reset_frame = true;
    }

    /* AMBIANT STRENGTH ON INPUT */
    if (key_state('j') || key_state('k'))
    {
        float old = ambiant_strength;
        if (key_state('j'))
            ambiant_strength = std::max(0.0f, ambiant_strength - 0.1f);
        else
            ambiant_strength = std::min(1.0f, ambiant_strength + 0.1f);

        clear_key_state('j');
        clear_key_state('k');

        if (old != ambiant_strength)
            reset_frame = true;
    }

    if (key_state('1'))
    {
        clear_key_state('1');
        toggle_Ldirect = !toggle_Ldirect;

        reset_frame = true;
    }

    if (key_state('2'))
    {
        clear_key_state('2');
        toggle_Lindirect = !toggle_Lindirect;

        reset_frame = true;
    }

    if (key_state('l'))
    {
        clear_key_state('l');

        if (toggle_Ldirect || toggle_Lindirect)
        {
            toggle_Ldirect = false;
            toggle_Lindirect = false;
        }
        else
        {
            toggle_Ldirect = true;
            toggle_Lindirect = true;
        }

        reset_frame = true;
    }

    int mx, my;
    unsigned int mb = SDL_GetRelativeMouseState(&mx, &my);
    if (mb & SDL_BUTTON(1))
        m_camera.rotation(mx, my);
    else if (mb & SDL_BUTTON(3))
        m_camera.move(mx);
    else if (mb & SDL_BUTTON(2))
        m_camera.translation((float)mx / (float)window_width(), (float)my / (float)window_height());

    return reset_frame || mb;
}


/* MAIN */

int main(int argc, char **argv)
{
    const char *filename = "tps/data/custom.obj";
    if (argc > 1)
        filename = argv[1];

    RT app(filename);
    app.run();

    return 0;
}
