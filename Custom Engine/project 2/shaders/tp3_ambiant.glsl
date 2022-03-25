#version 430

#ifdef COMPUTE_SHADER

/* UNIFORMS */

uniform int root;
uniform float ambiant_strength;
layout(binding = 1, r32ui)   coherent  uniform uimage2D     seeds;
layout(binding = 2, rgba32f) readonly  uniform image2DArray ray_image;
layout(binding = 3, rgba16)  writeonly uniform image2DArray color_image;


/* GLOBALS */

const float PI = 3.14159265359;
const float eps = 0.00001;
const float dist_max = 1000.0;
const int nb_ray_occ = 1;


/* STRUCTS */

struct Triangle
{
    vec3 a;		// sommet
    vec3 ab;	// arete 1
    vec3 ac;	// arete 2
    float padc;
    int mat_ind;
};

struct Material 
{
    vec4 emission;
    vec4 diffuse;
    vec4 specular;

    float ns() {return specular.a;}
};

struct SewedNode
{
    vec3 bounds_min;
    vec3 bounds_max;
    float padc;
    int next;
    int skip;
    int tr_id;

    bool leaf() { return tr_id != -1; }
};

struct RayHit
{
    vec3 o;
    vec3 d;
    int t_id;
    float t_max;
    float u;
    float v;
};


/* BUFFERS */

layout(std430, binding= 0) readonly buffer triangleData
{
    Triangle triangles[];
};

layout(std430, binding= 1) readonly buffer materialData 
{
    Material materials[];
};

layout(std430, binding = 3) readonly buffer sewedNodeData
{
    SewedNode sewed_nodes[];
};


/* RAY INTERSECTION */

bool intersect( const Triangle triangle,
                const vec3 o, const vec3 d, const float tmax,
                out float rt, out float ru, out float rv)
{
    vec3 pvec= cross(d, triangle.ac);
    float det= dot(triangle.ab, pvec);
    float inv_det= 1.0f / det;
    
    vec3 tvec= o - triangle.a;
    float u= dot(tvec, pvec) * inv_det;
    vec3 qvec= cross(tvec, triangle.ab);
    float v= dot(d, qvec) * inv_det;
    
    /* calculate t, ray intersects triangle */
    rt= dot(triangle.ac, qvec) * inv_det;
    ru= u;
    rv= v;

    // ne renvoie vrai que si l'intersection est valide : 
    // interieur du triangle, 0 < u < 1, 0 < v < 1, 0 < u+v < 1
    if(any(greaterThan(vec3(u, v, u+v), vec3(1, 1, 1))) || any(lessThan(vec2(u, v), vec2(0, 0))))
        return false;
    // comprise entre 0 et tmax du rayon
    return (rt < tmax && rt > 0);
}

RayHit get_ray(ivec2 xy)
{
    vec3 o = vec3(imageLoad(ray_image, ivec3(xy, 0)));
    vec3 d = vec3(imageLoad(ray_image, ivec3(xy, 1)));
    vec4 res = imageLoad(ray_image, ivec3(xy, 2));
    return RayHit(o, d, int(res.a), res.x, res.y, res.z);
}

bool hit(const RayHit ray, out vec3 p, out vec3 pn, out Triangle tr, out Material mat)
{
    if (ray.t_id == -1)
        return false;

    tr = triangles[ray.t_id];
    mat = materials[tr.mat_ind];

    p = tr.a + ray.u * tr.ab + ray.v * tr.ac;
    pn = normalize(cross(tr.ab, tr.ac));
    if (dot(ray.d, pn) > 0)
        pn = -pn;
    p += eps * pn;

    return true;
}

void swap(inout float x, inout float y)
{
    float c = x;
    x = y;
    y = c;
}

bool box_intersect(const vec3 box_min, const vec3 box_max, const RayHit ray)
{
    vec3 invd = 1.0 / ray.d;
    vec3 rmin = box_min;
    vec3 rmax = box_max;
    if (ray.d.x < 0)
        swap(rmax.x, rmin.x);
    if (ray.d.y < 0)
        swap(rmax.y, rmin.y);
    if (ray.d.z < 0)
        swap(rmax.z, rmin.z);
    vec3 dmin = (rmin - ray.o) * invd;
    vec3 dmax = (rmax - ray.o) * invd;

    float tmin = max(dmin.z, max(dmin.y, max(dmin.x, 0.0)));
    float tmax = min(dmax.z, min(dmax.y, min(dmax.x, ray.t_max)));
    return tmin <= tmax;
}

bool any_intersect(const RayHit ray)
{
    int index = root;
    while (index != -1)
    {
        const SewedNode node = sewed_nodes[index];
        if (node.leaf())
        {
            float t, u, v;
            if(intersect(triangles[node.tr_id], ray.o, ray.d, ray.t_max, t, u, v))
                return true;

            index = node.skip;
        }
        else
        {
            if (box_intersect(node.bounds_min, node.bounds_max, ray))
                index = node.next;
            else
                index = node.skip;
        }
    }

    return false;
}

/* AXIS FONCTIONS */

void axis(vec3 n, out vec3 t, out vec3 b) {
    float sign = n.z < 0 ? -1 : 1;
    float a = -1.0 / (sign + n.z);
    float d = n.x * n.y * a;
    t = vec3(1.0 + sign * n.x * n.x * a, sign * d, -sign * n.x);
    b = vec3(d, sign + n.y * n.y * a, -n.y);
}

vec3 local_to_tbn(const in vec3 local, vec3 t, vec3 b, vec3 n) {
    return local.x * t + local.y * b + local.z * n;
}


/* RNG */

const uint a_rng= 1103515245;
const uint b_rng= 12345;
const uint m_rng= 1u << 31;

float rng_sample(inout uint x) {
    x = (a_rng * x + b_rng) % m_rng;
    return float(x) / float(m_rng);
}

vec3 sample35(const float u1, const float u2)
{
    float cos_theta = sqrt(u1);
    float phi = 2.0 * PI * u2;

    float sin_theta = sqrt(1.0 - cos_theta * cos_theta);
    return vec3(cos(phi) * sin_theta, sin(phi) * sin_theta, cos_theta);
}

float pdf35(const vec3 w)
{
    if (w.z < 0)
        return 0;
    return w.z / PI;
}

/* MAIN */

layout( local_size_x= 8, local_size_y= 8 ) in;
void main( )
{
    ivec2 xy = ivec2(gl_GlobalInvocationID.xy);
    uint x_rng = imageLoad(seeds, xy).x;
    RayHit ray = get_ray(xy);

    vec3 ambiant = vec3(0.0, 0.0, 0.0);

    Triangle tr;
    Material mat;
    vec3 p;
    vec3 pn;

    if (hit(ray, p, pn, tr, mat))
    {
        vec3 axis_tt;
        vec3 axis_bb;
        axis(pn, axis_tt, axis_bb);

        /* AMBIANT */
        for (int i = 0; i < nb_ray_occ; ++i) {
            float u1 = rng_sample(x_rng);
            float u2 = rng_sample(x_rng);
            vec3 random_ray = sample35(u1, u2);
            vec3 ray_p = local_to_tbn(random_ray, axis_tt, axis_bb, pn);

            if (any_intersect(RayHit(p, ray_p, -1, dist_max, 0.0, 0.0)))
                continue;

            float cos_theta = abs(dot(ray_p, pn));
            float inv_pdf = 1.0/pdf35(random_ray);

            ambiant += (1.0 / PI) * cos_theta * mat.diffuse.rgb * inv_pdf;
        }
        ambiant = ambiant / float(nb_ray_occ);
    }

    imageStore(color_image, ivec3(xy, 3), vec4(ambiant * ambiant_strength, 1));
    imageStore(seeds, xy, uvec4(x_rng, 0, 0, 0));
}

#endif

