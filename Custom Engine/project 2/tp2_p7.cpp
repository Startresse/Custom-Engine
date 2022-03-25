
#include <cfloat>
#include <chrono>
#include <random>

#include "mesh.h"
#include "orbiter.h"
#include "vec.h"
#include "wavefront.h"

#include "image.h"
#include "image_hdr.h"
#include "image_io.h"

/* STRUCTURES */

struct RayHit
{
    Point o;         // origine
    float t;         // p(t)= o + td, position du point d'intersection sur le rayon
    Vector d;        // direction
    int triangle_id; // indice du triangle dans le mesh
    float u, v;
    int x, y;

    RayHit(const Point &_o, const Point &_e) : o(_o), t(1), d(Vector(_o, _e)), triangle_id(-1), u(), v(), x(), y() {}
    RayHit(const Point &_o, const Point &_e, const int _x, const int _y) : o(_o), t(1), d(Vector(_o, _e)), triangle_id(-1), u(), v(), x(_x), y(_y) {}
    operator bool() { return (triangle_id != -1); }

    RayHit() : triangle_id(-1) {}

    Point point()
    {
        return o + t * d;
    }
};

// renvoie la normale interpolee d'un triangle.
Vector normal(const RayHit &ray, const TriangleData &triangle)
{
    return normalize((1 - ray.u - ray.v) * Vector(triangle.na) + ray.u * Vector(triangle.nb) + ray.v * Vector(triangle.nc));
}

struct BBoxHit
{
    float tmin, tmax;

    BBoxHit() : tmin(FLT_MAX), tmax(-FLT_MAX) {}
    BBoxHit(const float _tmin, const float _tmax) : tmin(_tmin), tmax(_tmax) {}
    float centroid() const { return (tmin + tmax) / 2; }
    operator bool() const { return tmin <= tmax; }
};

struct BBox
{
    Point pmin, pmax;

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

    BBoxHit intersect(const RayHit &ray) const
    {
        Vector invd = Vector(1 / ray.d.x, 1 / ray.d.y, 1 / ray.d.z);
        return intersect(ray, invd);
    }

    BBoxHit intersect(const RayHit &ray, const Vector &invd) const
    {
        Point rmin = pmin;
        Point rmax = pmax;
        if (ray.d.x < 0)
            std::swap(rmin.x, rmax.x);
        if (ray.d.y < 0)
            std::swap(rmin.y, rmax.y);
        if (ray.d.z < 0)
            std::swap(rmin.z, rmax.z);
        Vector dmin = (rmin - ray.o) * invd;
        Vector dmax = (rmax - ray.o) * invd;

        float tmin = std::max(dmin.z, std::max(dmin.y, std::max(dmin.x, 0.f)));
        float tmax = std::min(dmax.z, std::min(dmax.y, std::min(dmax.x, ray.t)));
        return BBoxHit(tmin, tmax);
    }
};

struct Triangle
{
    Point p;
    Vector e1, e2;
    int id;

    Triangle(const Point &_a, const Point &_b, const Point &_c, const int _id) : p(_a), e1(Vector(_a, _b)), e2(Vector(_a, _c)), id(_id) {}

    void intersect(RayHit &ray) const
    {
        Vector pvec = cross(ray.d, e2);
        float det = dot(e1, pvec);

        float inv_det = 1 / det;
        Vector tvec(p, ray.o);

        float u = dot(tvec, pvec) * inv_det;
        if (u < 0 || u > 1)
            return;

        Vector qvec = cross(tvec, e1);
        float v = dot(ray.d, qvec) * inv_det;
        if (v < 0 || u + v > 1)
            return;

        float t = dot(e2, qvec) * inv_det;
        if (t < 0 || t > ray.t)
            return;

        ray.triangle_id = id;
        ray.t = t;
        ray.u = u;
        ray.v = v;
    }

    BBox bounds() const
    {
        BBox box(p);
        return box.insert(p + e1).insert(p + e2);
    }
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
    int root;

    int build(const Mesh &mesh)
    {
        std::vector<Triangle> trs;
        for (int i = 0; i < mesh.triangle_count(); ++i)
        {
            const TriangleData &trd = mesh.triangle(i);
            trs.emplace_back(trd.a, trd.b, trd.c, i);
        }

        BBox box;
        mesh.bounds(box.pmin, box.pmax);

        return build(box, trs);
    }

    // construit un bvh pour l'ensemble de triangles
    int build(const BBox &_bounds, const std::vector<Triangle> &_triangles)
    {
        triangles = _triangles; // copie les triangles pour les trier
        nodes.clear();          // efface les noeuds
        nodes.reserve(triangles.size());

        // construit l'arbre...
        root = build(_bounds, 0, triangles.size());
        // et renvoie la racine
        return root;
    }

    void intersect(RayHit &ray) const
    {
        Vector invd = Vector(1 / ray.d.x, 1 / ray.d.y, 1 / ray.d.z);
        intersect(root, ray, invd);
    }

    void intersect_fast(RayHit &ray) const
    {
        Vector invd = Vector(1 / ray.d.x, 1 / ray.d.y, 1 / ray.d.z);
        intersect_fast(root, ray, invd);
    }

    bool any_intersect(const RayHit &ray) const
    {
        RayHit r = ray;
        Vector invd = Vector(1 / ray.d.x, 1 / ray.d.y, 1 / ray.d.z);
        return any_intersect(root, r, invd);
    }

protected:
    int build(const BBox &bounds, const int begin, const int end)
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

    BBox triangle_bounds(const int begin, const int end)
    {
        BBox bbox = triangles[begin].bounds();
        for (int i = begin + 1; i < end; i++)
            bbox.insert(triangles[i].bounds());

        return bbox;
    }

    void intersect(const int index, RayHit &ray, const Vector &invd) const
    {
        const Node &node = nodes[index];
        if (node.bounds.intersect(ray, invd))
        {
            if (node.leaf())
            {
                for (int i = node.leaf_begin(); i < node.leaf_end(); i++)
                    triangles[i].intersect(ray);
            }
            else // if(node.internal())
            {
                intersect(node.internal_left(), ray, invd);
                intersect(node.internal_right(), ray, invd);
            }
        }
    }

    bool any_intersect(const int index, RayHit &ray, const Vector &invd) const
    {
        const Node &node = nodes[index];
        if (node.bounds.intersect(ray, invd))
        {
            if (node.leaf())
            {
                for (int i = node.leaf_begin(); i < node.leaf_end(); ++i)
                {
                    triangles[i].intersect(ray);
                    if (ray)
                        return true;
                }
            }
            else
            {
                return (any_intersect(node.internal_left(), ray, invd) ||
                        any_intersect(node.internal_right(), ray, invd));
            }
        }
        return false;
    }

    void intersect_fast(const int index, RayHit &ray, const Vector &invd) const
    {
        const Node &node = nodes[index];
        if (node.leaf())
        {
            for (int i = node.leaf_begin(); i < node.leaf_end(); i++)
                triangles[i].intersect(ray);
        }
        else // if(node.internal())
        {
            const Node &left_node = nodes[node.left];
            const Node &right_node = nodes[node.right];

            BBoxHit left = left_node.bounds.intersect(ray, invd);
            BBoxHit right = right_node.bounds.intersect(ray, invd);
            if (left && right) // les 2 fils sont touches par le rayon...
            {
                if (left.centroid() < right.centroid()) // parcours de gauche a droite
                {
                    intersect_fast(node.internal_left(), ray, invd);
                    intersect_fast(node.internal_right(), ray, invd);
                }
                else // parcours de droite a gauche
                {
                    intersect_fast(node.internal_right(), ray, invd);
                    intersect_fast(node.internal_left(), ray, invd);
                }
            }
            else if (left) // uniquement le fils gauche
                intersect_fast(node.internal_left(), ray, invd);
            else if (right)
                intersect_fast(node.internal_right(), ray, invd); // uniquement le fils droit
        }
    }
};

/* LIGHT SOURCES */

struct Source
{
    Point a, b, c;
    Color emission;
    Vector n;
    float area;

    Source() : a(), b(), c(), emission(), n(), area() {}

    Source(const TriangleData &data, const Color &color) : a(data.a), b(data.b), c(data.c), emission(color)
    {
        // normale geometrique du triangle abc, produit vectoriel des aretes ab et ac
        Vector ng = cross(Vector(a, b), Vector(a, c));
        n = normalize(ng);
        area = length(ng) / 2;
    }

    Point sample(const float u1, const float u2) const
    {
        // cf GI compemdium eq 18
        float r1 = std::sqrt(u1);
        float alpha = 1 - r1;
        float beta = (1 - u2) * r1;
        float gamma = u2 * r1;
        return alpha * a + beta * b + gamma * c;
    }

    float pdf(const Point &p) const
    {
        // todo : devrait renvoyer 0 pour les points a l'exterieur du triangle...
        return 1.f / area;
    }
};

struct Sources
{
    std::vector<Source> sources;
    float emission; // emission totale des sources
    float area;     // aire totale des sources

    Sources(const Mesh &mesh) : sources()
    {
        build(mesh);

        printf("%d sources\n", int(sources.size()));
        assert(sources.size());
    }

    void build(const Mesh &mesh)
    {
        area = 0;
        emission = 0;
        sources.clear();
        for (int id = 0; id < mesh.triangle_count(); id++)
        {
            const TriangleData &data = mesh.triangle(id);
            const Material &material = mesh.triangle_material(id);
            if (material.emission.power() > 0)
            {
                Source source(data, material.emission);
                emission = emission + source.area * source.emission.power();
                area = area + source.area;

                sources.push_back(source);
            }
        }
        struct
        {
            bool operator()(const Source &a, const Source &b) { return a.area > b.area; }
        } custom_more_area;
        std::sort(sources.begin(), sources.end(), custom_more_area);
    }

    int size() const { return int(sources.size()); }
    const Source &operator()(const int id) const { return sources[id]; }

    Source rnd_source_naive(float u) {return sources[u * size()];}
    Source rnd_source(std::uniform_real_distribution<float> &u01,
                      std::default_random_engine &rng)
    {
        float rnd = u01(rng);
        float cumul_area = 0;
        for (auto x : sources)
        {
            cumul_area += x.area / area;
            if (rnd < cumul_area)
                return x;
        }
        return sources[size() - 1];
    }
};

/* UTILITY */

struct World
{
    World(const Vector &_n) : n(_n)
    {
        float sign = std::copysign(1.0f, n.z);
        float a = -1.0f / (sign + n.z);
        float d = n.x * n.y * a;
        t = Vector(1.0f + sign * n.x * n.x * a, sign * d, -sign * n.x);
        b = Vector(d, sign + n.y * n.y * a, -n.y);
    }

    // transforme le vecteur du repere local vers le repere du monde
    Vector operator()(const Vector &local) const { return local.x * t + local.y * b + local.z * n; }

    // transforme le vecteur du repere du monde vers le repere local
    Vector inverse(const Vector &global) const { return Vector(dot(global, t), dot(global, b), dot(global, n)); }

    Vector t;
    Vector b;
    Vector n;
};

/* "POST PROCESSING" */

void gamma_correction(Color &c)
{
    c.r = pow(c.r, 1.f / 2.2);
    c.g = pow(c.g, 1.f / 2.2);
    c.b = pow(c.b, 1.f / 2.2);
}

Image filtre(const Image &image)
{
    Image res(image.width(), image.height());

    int size_kernel = 5;
    double kernel[5][5] = {
        {
            1,
            4,
            6,
            4,
            1,
        },
        {
            4,
            16,
            24,
            16,
            4,
        },
        {
            6,
            24,
            36,
            24,
            6,
        },
        {
            4,
            16,
            24,
            16,
            4,
        },
        {
            1,
            4,
            6,
            4,
            1,
        },
    };
    int off_h = size_kernel / 2;
    int off_v = size_kernel / 2;

#pragma omp parallel for schedule(dynamic, 1)
    for (int py = off_h; py < image.height() - off_h; py++)
    {
        for (int px = off_v; px < image.width() - off_v; px++)
        {
            Color sum = Black();
            for (int u = 0; u < size_kernel; ++u)
            {
                for (int v = 0; v < size_kernel; ++v)
                {
                    Color pv = image(px - off_h + u, py - off_v + v);
                    if (std::isnan(pv.r) || std::isnan(pv.g) || std::isnan(pv.b) ||
                        std::isinf(pv.r) || std::isinf(pv.g) || std::isinf(pv.b))
                        continue;
                    sum = sum +
                          (1.0f / 256.0f) * kernel[u][v] * pv;
                }
            }
            res(px, py) = sum;
        }
    }

    return res;
}

/* RANDOM SAMPLING */

Vector sample34(const float u1, const float u2)
{
    // coordonnees theta, phi
    float cos_theta = u1;
    float phi = float(2 * M_PI) * u2;

    // passage vers x, y, z
    float sin_theta = std::sqrt(1 - cos_theta * cos_theta);
    return Vector(std::cos(phi) * sin_theta, std::sin(phi) * sin_theta, cos_theta);
}

float pdf34(const Vector &w)
{
    if (w.z < 0)
        return 0;
    return 1 / float(2 * M_PI);
}

Vector sample35(const float u1, const float u2)
{
    // coordonnees theta, phi
    float cos_theta = std::sqrt(u1);
    float phi = float(2 * M_PI) * u2;

    // passage vers x, y, z
    float sin_theta = std::sqrt(1 - cos_theta * cos_theta);
    return Vector(std::cos(phi) * sin_theta, std::sin(phi) * sin_theta, cos_theta);
}

float pdf35(const Vector &w)
{
    if (w.z < 0)
        return 0;
    return w.z / float(M_PI);
}

/* UTILITY */

Color fr_calc(Point s, Point p, Point o, Vector pn, Material material, float K)
{
    float fr_d = K / float(M_PI);
    Vector h = normalize(normalize(Vector(p, o)) + normalize(Vector(p, s)));
    float cos_theta_h = std::max(0.0f, dot(pn, h));
    float alpha = material.ns;
    float fr_s = (1 - K) * ((alpha + 8) / (8 * M_PI)) * pow(cos_theta_h, alpha);
    return fr_d * material.diffuse + fr_s * material.specular;
}

/* MAIN */

int main(const int argc, const char **argv)
{
    /* SETUP */

    const char *mesh_filename = "tps/data/cornell_extra.obj";
    const char *orbiter_filename = "tps/data/cornell_orbiter.txt";

    if (argc > 1)
        mesh_filename = argv[1];
    if (argc > 2)
        orbiter_filename = argv[2];

    printf("%s: '%s' '%s'\n", argv[0], mesh_filename, orbiter_filename);

    const int res_x = 1240;
    const int res_y = 920;
    Image image(res_x, res_y);
    Image L1_img(res_x, res_y);
    Image L2_img(res_x, res_y);
    Image Le_img(res_x, res_y);
    Image Locc_img(res_x, res_y);
    std::vector<std::vector<RayHit>> ray_img(res_y, std::vector<RayHit>(res_x));

    /* TRANSFORMS */
    Orbiter camera;
    if (camera.read_orbiter(orbiter_filename))
        return 1;

    Transform model = Identity();
    Transform view = camera.view();
    Transform projection = camera.projection(image.width(), image.height(), 45);
    Transform viewport = Viewport(image.width(), image.height());

    Transform full = viewport * projection * view * model;
    Transform inv = Inverse(full);

    /* INIT */

    Mesh mesh = read_mesh(mesh_filename);
    if (mesh.triangle_count() == 0)
        return 1;

    BVH bvh;
    bvh.build(mesh);
    Sources sources(mesh);

    /* PARAMETERS */
    const int inf_mesh = 1000000;
    const float epsilon = 0.0001;

    const uint nb_dir_L1 = 16;
    const uint nb_dir_L2 = 16;
    const uint attenuate_L1_L2 = 8; // divides the number of L1 ray in L2
    const uint nb_dir_Locc = 512;

    const bool Le_toggle = true;
    const bool L1_toggle = true;
    const bool L2_toggle = true;
    const bool Locc_toggle = false;

    const float Le_val_fact = 1.0f;
    const float L1_val_fact = 0.7f;
    const float L2_val_fact = 0.5;
    const float Locc_val_fact = 1.0f;
    const float K = 0.7f;
    const uint nb_accu = 2;


    assert(K <= 1.0f && K >= 0.0f);
    const uint nb_dir_L1_L2 = nb_dir_L1 / attenuate_L1_L2;

    /* UTILITY */
    auto L1_calc = [&](const Point &p,
                       const Vector &pn,
                       const RayHit &ray,
                       const Material &material,
                       uint nb_ray,
                       std::uniform_real_distribution<float> &u01,
                       std::default_random_engine &rng) {
        Color L1 = Black();
        for (uint i = 0; i < nb_ray; ++i)
        {
            /* Random sample on source */
            // const Source &src = sources.rnd_source_naive(u01(rng));
            const Source &src = sources.rnd_source(u01, rng);
            const Point &point_src = src.sample(u01(rng), u01(rng));
            float pdf = src.pdf(point_src) * (src.area / sources.area);
            float inv_pdf;
            if (pdf != 0)
                inv_pdf = 1.0f / pdf;
            else
                inv_pdf = 0;

            /* V(sk, p) */
            Point ps = point_src + epsilon * src.n;
            RayHit p_to_s = RayHit(p, ps);
            if (bvh.any_intersect(p_to_s))
                continue;

            /* Le(sk, p) */
            Color LeSk = src.emission;

            /* fr(sk, p, o) */
            Color fr = fr_calc(point_src, p, ray.o, pn, material, K);

            /* costhet */
            Vector dps = normalize(p_to_s.d);
            float cos_theta_1 = std::max(0.0f, dot(pn, dps));
            float cos_theta_s = std::max(0.0f, dot(src.n, -dps));
            float d2 = length2(Vector(p, point_src));
            float cos_factor;
            if (d2 < epsilon)
                cos_factor = 0;
            else
                cos_factor = cos_theta_1 * cos_theta_s / d2;

            L1 = L1 + LeSk * fr * cos_factor * inv_pdf;
        }
        if (nb_ray > 0)
            L1 = L1 / nb_ray;
        return L1;
    };

    /* APPLICATION */
    auto cpu_start = std::chrono::high_resolution_clock::now();

    for (uint anti_alias_acc = 0; anti_alias_acc < nb_accu; ++anti_alias_acc)
    {
        std::cout << std::endl;
        std::cout << "IMAGE " << anti_alias_acc + 1<< "/" << nb_accu;
        std::cout << std::endl;

        std::cout << ">";
        if (Le_toggle)
            std::cout << " Le";
        if (L1_toggle || L2_toggle)
            std::cout << " L1";
        if (Locc_toggle)
            std::cout << " Locc";
        std::cout << "..." << std::endl;
#pragma omp parallel for schedule(dynamic, 1)
        for (int py = 0; py < image.height(); py++)
        {
            std::random_device seed;
            std::default_random_engine rng(seed());
            std::uniform_real_distribution<float> u01(0.f, 1.f);

            for (int px = 0; px < image.width(); px++)
            {
                float x = px + u01(rng);
                float y = py + u01(rng);

                Point o = inv(Point(x, y, 0));
                Point e = inv(Point(x, y, 1));

                RayHit ray = RayHit(o, e);
                bvh.intersect(ray);
                ray_img[py][px] = ray;
                if (!ray)
                    continue;

                const TriangleData &triangle = mesh.triangle(ray.triangle_id);
                const Material &material = mesh.triangle_material(ray.triangle_id);

                Point p = ray.point();
                Vector pn = normal(ray, triangle);
                if (dot(pn, ray.d) > 0)
                    pn = -pn;
                p = p + epsilon * pn;

                World w(pn);

                /* Le */
                if (Le_toggle)
                {
                    Color Le = material.emission;
                    Le_img(px, py) = Le;
                }

                /* L1 */
                if (L1_toggle || L2_toggle)
                {
                    L1_img(px, py) = L1_calc(p, pn, ray, material, nb_dir_L1, u01, rng);
                }

                /* Ambiant Occlusion */
                if (Locc_toggle)
                {
                    Color L_occ = Black();
                    for (uint i = 0; i < nb_dir_Locc; ++i)
                    {
                        /* random direction */
                        Vector vk_undirected = sample35(u01(rng), u01(rng));
                        Vector vk = w(vk_undirected);
                        float inv_pdf = 1.0 / pdf35(vk_undirected);

                        /* V(sk, p) */
                        RayHit rh = RayHit(p, Point(p + inf_mesh * vk));
                        if (bvh.any_intersect(rh))
                            continue;

                        /* costhet */
                        float cos_theta_k = std::max(0.0f, dot(vk, pn));

                        L_occ = L_occ + (1.0 / M_PI) * cos_theta_k * inv_pdf * material.diffuse;
                    }
                    if (nb_dir_Locc > 0.0)
                        L_occ = L_occ / nb_dir_Locc;
                    Locc_img(px, py) = L_occ;
                }
            }
        }
#pragma omp barrier

        if (L2_toggle)
            std::cout << "> L2..." << std::endl;
        /* L2 */
        // L2 is in second loop because it may use L1 results of different pixels,
        // so we ensure that they all are done calculating
        if (L2_toggle)
        {
#pragma omp parallel for schedule(dynamic, 1)
            for (int py = 0; py < image.height(); py++)
            {
                std::random_device seed;
                std::default_random_engine rng(seed());
                std::uniform_real_distribution<float> u01(0.f, 1.f);

                for (int px = 0; px < image.width(); px++)
                {
                    Color L2 = Black();

                    RayHit ray = ray_img[py][px];
                    if (!ray)
                        continue;

                    const TriangleData &triangle = mesh.triangle(ray.triangle_id);
                    const Material &material = mesh.triangle_material(ray.triangle_id);

                    Point p = ray.point();
                    Vector pn = normal(ray, triangle);
                    if (dot(pn, ray.d) > 0)
                        pn = -pn;
                    p = p + epsilon * pn;

                    World w(pn);

                    for (uint i = 0; i < nb_dir_L2; ++i)
                    {
                        /* Random vector */
                        Vector vk_undirected = sample35(u01(rng), u01(rng));
                        Vector vk = w(vk_undirected);
                        float inv_pdf = 1.0 / pdf35(vk_undirected);

                        RayHit rh = RayHit(p, Point(p + inf_mesh * vk));
                        bvh.intersect(rh);
                        if (!rh)
                            continue;

                        const TriangleData &q_triangle = mesh.triangle(rh.triangle_id);
                        const Material &q_material = mesh.triangle_material(rh.triangle_id);

                        /* Ldirect(q, p) */
                        // q is the point of second ray
                        Point q = rh.point();
                        Vector qn = normal(rh, q_triangle);
                        if (dot(qn, rh.d) > 0)
                            qn = -qn;
                        q = q + epsilon * qn;
                        // proj is the point in the image
                        Point proj = full(q);
                        // if the point q can be seen by the camera, L1 is already
                        // calculated in L1_img so we can use it, otherwise we
                        // need to calculate it
                        RayHit ray_to_q(ray.o, q);
                        bvh.intersect(ray_to_q);
                        Color L1 = Black();
                        if (length(ray_to_q.point() - q) < epsilon)
                            L1 = L1_img(proj.x, proj.y);
                        else
                            L1 = L1_calc(q, qn, rh, q_material, nb_dir_L1_L2, u01, rng);

                        /* fr(q, p, o) */
                        Color fr = fr_calc(q, p, ray.o, pn, material, K);

                        /* costheta */
                        float cos_theta = std::max(0.0f, dot(vk, pn));

                        L2 = L2 + L1 * fr * cos_theta * inv_pdf;
                    }
                    if (nb_dir_L2 > 0.0)
                        L2 = L2 / nb_dir_L2;

                    L2_img(px, py) = L2;
                }
            }
        }

        std::cout << "> image accumulation..." << std::endl;
        // coloring output image
#pragma omp parallel for schedule(dynamic, 1)
        for (int py = 0; py < image.height(); py++)
        {
            for (int px = 0; px < image.width(); px++)
            {
                Color color = Black();
                if (Le_toggle)
                    color = color + Le_val_fact * Le_img(px, py);
                if (L1_toggle)
                    color = color + L1_val_fact * L1_img(px, py);
                if (L2_toggle)
                    color = color + L2_val_fact * L2_img(px, py);
                if (Locc_toggle)
                    color = color + Locc_val_fact * Locc_img(px, py);

                gamma_correction(color);

                image(px, py) = image(px, py) + Color(color, 1);
            }
        }
        std::cout << "DONE!" << std::endl;
        auto cpu_stop = std::chrono::high_resolution_clock::now();
        int cpu_time = std::chrono::duration_cast<std::chrono::milliseconds>(cpu_stop - cpu_start).count();
        std::cout << "remaining : ";
        std::cout << int(cpu_time / 1000) * (nb_accu - (anti_alias_acc + 1))/(float)(anti_alias_acc + 1);
        std::cout << "s" << std::endl;
    }

    std::cout << "\n\nFINAL ACCUMULATION..." << std::endl;
#pragma omp parallel for schedule(dynamic, 1)
    for (int py = 0; py < image.height(); py++)
    {
        for (int px = 0; px < image.width(); px++)
        {
            image(px, py) = image(px, py) / nb_accu;
        }
    }
    std::cout << "DONE!" << std::endl << std::endl;

    /* POST PROCESSING */
    // image = filtre(image);

    /* TIME DISPLAY */

    auto cpu_stop = std::chrono::high_resolution_clock::now();
    int cpu_time = std::chrono::duration_cast<std::chrono::milliseconds>(cpu_stop - cpu_start).count();
    printf("cpu  %ds %03dms\n", int(cpu_time / 1000), int(cpu_time % 1000));

    /* DATA EXPORT */

    write_image(image, "tps/res/render.png");
    write_image_hdr(image, "tps/res/render.hdr");

    return 0;
}
