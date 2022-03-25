#version 430

#ifdef COMPUTE_SHADER


/* UNIFORMS */

uniform int root;
uniform mat4 invMatrix;
layout(binding = 1, r32ui)   coherent  uniform uimage2D     seeds;
layout(binding = 2, rgba32f) writeonly uniform image2DArray ray_image;


/* STRUCTS */

struct Triangle
{
    vec3 a;		// sommet
    vec3 ab;	// arete 1
    vec3 ac;	// arete 2
    float padc;
    int mat_ind;
    int tr_id;
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

layout(std430, binding = 0) readonly buffer triangleData
{
    Triangle triangles[];
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
    vec3 pvec = cross(d, triangle.ac);
    float det = dot(triangle.ab, pvec);
    float inv_det = 1.0f / det;
    
    vec3 tvec = o - triangle.a;
    float u = dot(tvec, pvec) * inv_det;
    vec3 qvec = cross(tvec, triangle.ab);
    float v = dot(d, qvec) * inv_det;
    
    /* calculate t, ray intersects triangle */
    rt = dot(triangle.ac, qvec) * inv_det;
    ru = u;
    rv = v;

    // ne renvoie vrai que si l'intersection est valide : 
    // interieur du triangle, 0 < u < 1, 0 < v < 1, 0 < u+v < 1
    if(any(greaterThan(vec3(u, v, u+v), vec3(1, 1, 1))) || any(lessThan(vec2(u, v), vec2(0, 0))))
        return false;
    // comprise entre 0 et tmax du rayon
    return (rt < tmax && rt > 0);
}

void intersect(int i, inout RayHit ray)
{
    float t, u, v;
    if (intersect(triangles[i], ray.o, ray.d, ray.t_max, t, u, v))
    {
        ray.t_max = t;
        ray.u = u;
        ray.v = v;
        ray.t_id = i;
    }
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

void intersect(inout RayHit ray)
{
    int index = root;
    while (index != -1)
    {
        const SewedNode node = sewed_nodes[index];
        if (node.leaf())
        {
            intersect(node.tr_id, ray);
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
}


/* RNG */

const uint a_rng= 1103515245;
const uint b_rng= 12345;
const uint m_rng= 1u << 31;

float rng_sample(inout uint x) {
    x = (a_rng * x + b_rng) % m_rng;
    return float(x) / float(m_rng);
}


/* MAIN */

layout( local_size_x = 8, local_size_y = 8 ) in;
void main( )
{
    ivec2 xy = ivec2(gl_GlobalInvocationID.xy);
    uint x_rng = imageLoad(seeds, xy).x;
    vec2 position = vec2(xy);

    position.x += rng_sample(x_rng) - 0.5;
    position.y += rng_sample(x_rng) - 0.5;
    vec4 oh = invMatrix * vec4(position, 0, 1);
    vec4 eh = invMatrix * vec4(position, 1, 1);

    vec3 o = oh.xyz / oh.w;
    vec3 d = eh.xyz / eh.w - oh.xyz / oh.w;
    float far = length(d);
    d = normalize(d);

    RayHit ray = RayHit(o, d, -1, far, 0.0, 0.0);

    intersect(ray);

    imageStore(ray_image, ivec3(xy, 0), vec4(o, 0.0));
    imageStore(ray_image, ivec3(xy, 1), vec4(d, 0.0));
    imageStore(ray_image, ivec3(xy, 2), vec4(ray.t_max, ray.u, ray.v, ray.t_id));
    imageStore(seeds, xy, uvec4(x_rng, 0, 0, 0));
}

#endif

