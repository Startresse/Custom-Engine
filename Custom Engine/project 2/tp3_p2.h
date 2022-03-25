#pragma once

#include <algorithm>
#include <cfloat>
#include <cmath>
#include <random>

#include "app_time.h"
#include "color.h"
#include "mat.h"
#include "mesh.h"
#include "orbiter.h"
#include "program.h"
#include "uniforms.h"
#include "vec.h"
#include "wavefront.h"

namespace glsl
{
    template <typename T>
    struct alignas(8) gvec2
    {
        alignas(4) T x, y;

        gvec2() {}
        gvec2(const vec2 &v) : x(v.x), y(v.y) {}
    };

    typedef gvec2<float> vec2;
    typedef gvec2<int> ivec2;
    typedef gvec2<unsigned int> uvec2;
    typedef gvec2<int> bvec2;

    template <typename T>
    struct alignas(16) gvec3
    {
        alignas(4) T x, y, z;

        gvec3() {}
        gvec3(const vec3 &v) : x(v.x), y(v.y), z(v.z) {}
        gvec3(const Point &v) : x(v.x), y(v.y), z(v.z) {}
        gvec3(const Vector &v) : x(v.x), y(v.y), z(v.z) {}
    };

    typedef gvec3<float> vec3;
    typedef gvec3<int> ivec3;
    typedef gvec3<unsigned int> uvec3;
    typedef gvec3<int> bvec3;

    template <typename T>
    struct alignas(16) gvec4
    {
        alignas(4) T x, y, z, w;

        gvec4() {}
        gvec4(const vec4 &v) : x(v.x), y(v.y), z(v.z), w(v.w) {}
    };

    typedef gvec4<float> vec4;
    typedef gvec4<int> ivec4;
    typedef gvec4<unsigned int> uvec4;
    typedef gvec4<int> bvec4;
} // namespace glsl

struct BBox
{
    alignas(16) Point pmin;
    alignas(16) Point pmax;

    BBox() : pmin(), pmax() {}

    BBox(const Point &p) : pmin(p), pmax(p) {}
    BBox(const BBox &box) : pmin(box.pmin), pmax(box.pmax) {}

    BBox &insert(const Point &p)
    {
        pmin = min(pmin, p);
        pmax = max(pmax, p);
        return *this;
    }
    BBox &insert(const BBox &box)
    {
        pmin = min(pmin, box.pmin);
        pmax = max(pmax, box.pmax);
        return *this;
    }

    float centroid(const int axis) const { return (pmin(axis) + pmax(axis)) / 2; }
};

Point operator+(const Point &p, const glsl::vec3 &v)
{
    return Point(p.x + v.x, p.y + v.y, p.z + v.z);
}

struct Triangle
{
    Point a;
    glsl::vec3 ab;
    glsl::vec3 ac;
    int mat_ind;
    int id;

    Triangle(const Point &_a, const Point &_b, const Point &_c, const int _mat_ind, const int _id)
        : a(_a), ab(_b - _a), ac(_c - _a), mat_ind(_mat_ind), id(_id) {}
    Triangle(const Point &_a, const Point &_b, const Point &_c, const int _id)
        : a(_a), ab(_b - _a), ac(_c - _a), mat_ind(0), id(_id) {}
    Triangle() : a(), ab(), ac(), mat_ind(0), id(0) {}

    BBox bounds() const
    {
        BBox box(a);
        return box.insert(a + ab).insert(a + ac);
    }
};

struct smallMat
{
    Color emission; //!< pour une source de lumiere
    Color diffuse;  //!< couleur diffuse
    Color specular; //!< couleur du reflet
};


/* BVH */

struct Node
{
    BBox bounds;
    int left;
    int right;

    bool internal() const { return right > 0; } // renvoie vrai si le noeud est un noeud interne
    int internal_left() const
    {
        assert(internal());
        return left;
    } // renvoie le fils gauche du noeud interne
    int internal_right() const
    {
        assert(internal());
        return right;
    } // renvoie le fils droit

    bool leaf() const { return right < 0; } // renvoie vrai si le noeud est une feuille
    int leaf_begin() const
    {
        assert(leaf());
        return -left;
    } // renvoie le premier objet de la feuille
    int leaf_end() const
    {
        assert(leaf());
        return -right;
    } // renvoie le dernier objet
};

struct SewedNode
{
    BBox bounds;
    int next;
    int skip;
    int tr_id;

    SewedNode() {}
    SewedNode(BBox b, int n, int s, int id) : bounds(b), next(n), skip(s), tr_id(id) {}
};

// creation d'un noeud interne
Node make_node(const BBox &bounds, const int left, const int right)
{
    Node node{bounds, left, right};
    assert(node.internal()); // verifie que c'est bien un noeud...
    return node;
}

// creation d'une feuille
Node make_leaf(const BBox &bounds, const int begin, const int end)
{
    Node node{bounds, -begin, -end};
    assert(node.leaf()); // verifie que c'est bien une feuille...
    return node;
}

struct triangle_less1
{
    int axis;
    float cut;

    triangle_less1(const int _axis, const float _cut) : axis(_axis), cut(_cut) {}

    bool operator()(const Triangle &triangle) const
    {
        // re-construit l'englobant du triangle
        BBox bounds = triangle.bounds();
        return bounds.centroid(axis) < cut;
    }
};

struct BVH
{
    std::vector<Node> nodes;
    std::vector<Triangle> triangles;
    std::vector<int> sources;
    std::vector<SewedNode> sewed_nodes;
    int root;

    int build(const Mesh &mesh);
    int build(const BBox &_bounds, const std::vector<Triangle> &_triangles);

    void sew() {sewed_nodes = sew(root, -1);}

protected:

    int build(const BBox &bounds, const int begin, const int end);
    int build_sources(const Mesh&);

    BBox triangle_bounds(const int begin, const int end);

    std::vector<SewedNode> sew(int node, int skip);

};



/* DISPLAY APP */

struct RT : public AppTime
{
    RT(const char *filename) : AppTime(1024, 640)
    {
        m_mesh = read_mesh(filename);
    }

    int init();
    void init_buffers();
    void init_textures();

    int quit();

    int render();
    bool handle_input();

protected:
    /* PARAMETERS */
    uint nb_max_frame = 1000;

    /* VARIABLES */
    uint frame = 0;
    float ambiant_strength = 0.1;
    bool toggle_Ldirect = true;
    bool toggle_Lindirect = true;
    Transform m;
    Transform v;
    Transform p;
    Transform im;
    Transform T;
    Transform T_inv;
    int root;


    /* UTIL */

    Mesh m_mesh;
    Orbiter m_camera;

    /* TEXTURES */
    GLuint resultat_tex;
    GLuint color_tex;
    GLuint ray_tex;
    GLuint seeds_tex;
    std::vector<std::pair<GLuint*, GLsizei>> textures = {
        std::make_pair(&resultat_tex,  1),
        std::make_pair(&color_tex,     1),
        std::make_pair(&ray_tex,       1),
        std::make_pair(&seeds_tex,     1),
    };

    /* BUFFER */
    GLuint triangle_buffer;
    GLuint sources_buffer;
    GLuint material_buffer;
    GLuint nodes_buffer;
    std::vector<std::pair<GLuint*, GLsizei>> buffers = {
        std::make_pair(&triangle_buffer, 1),
        std::make_pair(&sources_buffer,  1),
        std::make_pair(&material_buffer, 1),
        std::make_pair(&nodes_buffer,    1),
    };

    /* FRAMEBUFFER */
    GLuint blit_framebuffer;

    /* PROGRAM */
    GLuint intersect_shader;
    GLuint ambiant_shader;
    GLuint light_shader;
    GLuint L2_shader;
    GLuint accu_shader;
    std::vector<std::pair<GLuint*, const char*>> shaders = {
        std::make_pair(&intersect_shader, "tps/shaders/tp3_intersect.glsl"),
        std::make_pair(&ambiant_shader,   "tps/shaders/tp3_ambiant.glsl"),
        std::make_pair(&light_shader,     "tps/shaders/tp3_light.glsl"),
        std::make_pair(&L2_shader,        "tps/shaders/tp3_L2.glsl"),
        std::make_pair(&accu_shader,      "tps/shaders/tp3_accu.glsl"),
    };
};