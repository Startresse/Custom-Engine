#version 430

#ifdef COMPUTE_SHADER

/* UNIFORMS */

uniform int root;
layout(binding = 1, r32ui)   coherent  uniform uimage2D     seeds;
layout(binding = 2, rgba32f) readonly  uniform image2DArray ray_image;
layout(binding = 3, rgba16)  writeonly uniform image2DArray color_image;


/* GLOBALS */

const float PI = 3.14159265359;
const float eps = 0.00001;
const float K = 0.93;
const int nb_ray_l1 = 1;


/* STRUCTS */

struct Triangle
{
    vec3 a;		// sommet
    vec3 ab;	// arete 1
    vec3 ac;	// arete 2
    float padc;
    int mat_ind;

    vec3 get_sample( float u1, float u2 )
    {
        float r1 = sqrt(u1);
        float alpha = 1.0 - r1;
        float beta = (1.0 - u2) * r1;
        float gamma = u2 * r1;
        return alpha*a + beta*(a + ab) + gamma*(a + ac);
    }

    float pdf()
    {
        return 2.0 / length(cross(ab, ac));
    }

    vec3 n()
    {
        return normalize(cross(ab, ac));
    }
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

layout(std430, binding= 2) readonly buffer sourcesData 
{
    int sources[];
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


/* RNG */

const uint a_rng= 1103515245;
const uint b_rng= 12345;
const uint m_rng= 1u << 31;

float rng_sample(inout uint x) {
    x = (a_rng * x + b_rng) % m_rng;
    return float(x) / float(m_rng);
}


/* LIGHT UTILITY */

vec3 fr_calc(vec3 s, vec3 p, vec3 o, vec3 pn, Material mat)
{
    float fr_d = K / PI;
    vec3 h = normalize(normalize(o - p) + normalize(s - p));
    float cos_theta_h = max(0.0, dot(pn, h));
    float alpha = mat.ns();
    float fr_s = (1 - K) * ((alpha + 8) / (8 * PI)) * pow(cos_theta_h, alpha);
    return fr_d * mat.diffuse.rgb + fr_s * mat.specular.rgb;
}

vec3 L1_calc(
    inout uint x_rng,
    vec3 p,
    vec3 o,
    vec3 pn,
    Material mat
)
{
    vec3 L1 = vec3(0.0, 0.0, 0.0);

    for (int i = 0; i < sources.length(); i++)
    {
        Triangle src = triangles[sources[i]];
        Material mat_src = materials[src.mat_ind];
        for (int j = 0; j < nb_ray_l1; ++j)
        {
            float u1 = rng_sample(x_rng);
            float u2 = rng_sample(x_rng);
            vec3 p_src = src.get_sample(u1, u2);

            // V
            vec3 ps = p_src + eps*src.n();
            vec3 s_p = ps - p;
            vec3 dir = normalize(s_p);
            float hh = length(s_p);
            if(any_intersect(RayHit(p, dir, -1, hh, 0.0, 0.0)))
                continue;

            // Le
            vec3 Le = mat_src.emission.rgb;

            // fr
            vec3 fr = fr_calc(ps, p, o, pn, mat);

            // costhet
            float cos_theta_1 = dot(pn, dir);
            float cos_theta_s = max(0.0, dot(src.n(), -dir));
            float d2 = dot(s_p, s_p);
            float cos_factor = cos_theta_1 * cos_theta_s / d2;

            // pdf
            float inv_pdf = 1.0 / src.pdf();

            L1 = L1 + Le * fr * cos_factor * inv_pdf;
        }
    }
    L1 = L1 / float(nb_ray_l1 * sources.length());

    return L1;
}


/* MAIN */

layout( local_size_x= 8, local_size_y= 8 ) in;
void main( )
{
    ivec2 xy = ivec2(gl_GlobalInvocationID.xy);
    uint x_rng = imageLoad(seeds, xy).x;
    RayHit ray = get_ray(xy);

    vec3 L0 = vec3(0.0, 0.0, 0.0);
    vec3 L1 = vec3(0.0, 0.0, 0.0);

    Triangle tr;
    Material mat;
    vec3 p;
    vec3 pn;

    if (hit(ray, p, pn, tr, mat))
    {
        // L0
        L0 = mat.emission.rgb;

        // L1
        L1 = L1_calc(x_rng, p, ray.o, pn, mat);
    }

    imageStore(color_image, ivec3(xy, 0), vec4(L0, 1));
    imageStore(color_image, ivec3(xy, 1), vec4(L1, 1));
    imageStore(seeds, xy, uvec4(x_rng, 0, 0, 0));
}

#endif

