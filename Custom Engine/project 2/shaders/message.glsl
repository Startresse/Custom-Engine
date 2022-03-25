
#version 430

#ifdef COMPUTE_SHADER

#define M_PI 3.1415926535
#define PHI 1.61803398875

struct RNG
{
    uint x;
    uint x0;
    uint a;
    uint b;
    uint m;
};

RNG build_rng(uint seed, uint a, uint b, uint m)
{
    RNG rng;

    rng.x = seed;
    rng.x0 = seed;
    rng.a = a;
    rng.b = b;
    rng.m = m;

    return rng;
}

RNG sample_rng(RNG rng, out float rand) // renvoie un reel aleatoire dans [0 1]
{
    rng.x = (rng.a * rng.x + rng.b) % rng.m;
    rand = float(rng.x) / float(rng.m);
    return rng;
}

RNG index_rng(RNG rng, const uint i, out uint gen) // prepare la generation du terme i
{
    uint cur_mul = rng.a;
    uint cur_add = rng.b;
    uint acc_mul = 1u;
    uint acc_add = 0u;

    uint delta = i;
    while (delta != 0)
    {
        if((delta & 1u) == 1)
        {
            acc_mul = acc_mul * cur_mul;
            acc_add = acc_add * cur_mul + cur_add;
        }
        
        cur_add = cur_mul * cur_add + cur_add;
        cur_mul = cur_mul * cur_mul;
        delta = delta >> 1u;
    }
    
    rng.x = acc_mul * rng.x0 + acc_add;
    gen = rng.x;
    
    return rng;
}

struct Triangle
{
    vec3 a;
    vec3 ab;
    vec3 ac;
    vec3 diffuse;
};

// shader storage buffer 0
layout(std430, binding= 0) readonly buffer triangleData
{
    Triangle triangles[];
};

bool intersect( const Triangle triangle, const vec3 o, const vec3 d, const float tmax, out float rt, out float ru, out float rv )
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
    if (any(greaterThan(vec3(u, v, u+v), vec3(1, 1, 1))) || any(lessThan(vec2(u, v), vec2(0, 0))))
        return false;
        
    // comprise entre 0 et tmax du rayon
    return (rt < tmax && rt > 0);
}

vec3 sample35(float u1, float u2)
{
    // coordonnees theta, phi
    float cos_theta = sqrt(u1);
    float phi = float(2.0f * float(M_PI)) * u2;

    // passage vers x, y, z
    float sin_theta = sqrt(1.0f - cos_theta * cos_theta);
    return vec3(cos(phi) * sin_theta, sin(phi) * sin_theta, cos_theta);
}

float pdf35(vec3 v) {
    if (v.z < 0)
        return 0;
    return v.z / float(M_PI);
}

vec3 fibo(int j, int N, float rand)
{
    float cos_theta = 1.0f - (2.0f * j + 1.0f) / (2.0f * N);
    float sin_theta = sqrt(1.0f - pow(cos_theta, 2.0f));
    float j_on_phi = (j + rand) / PHI;
    float lil_phi = 2.0f * float(M_PI) * (j_on_phi - floor(j_on_phi));

    return vec3(cos(lil_phi)*sin_theta, sin(lil_phi)*sin_theta, cos_theta);
}

void world(const vec3 n, out vec3 t, out vec3 b) {
    float signe = n.z < 0.0f ? -1.0f : 1.0f; 
    float a = -1.0f / (signe + n.z);
    float d = n.x * n.y * a;

    t = vec3(1.0f + signe * n.x * n.x * a, signe * d, -signe * n.x);
    b = vec3(d, signe + n.y * n.y * a, -n.y);
}

vec3 local_to_global( const in vec3 local, const in vec3 t, const in vec3 b, const in vec3 n ) {
    return  local.x * t + local.y * b + local.z * n;
}

uniform mat4 invMatrix;

// image resultat
layout(binding= 0, rgba8)  writeonly uniform image2D image;

// 8x8 threads
layout( local_size_x= 8, local_size_y= 8 ) in;
void main( )
{
    RNG rng = build_rng(1, 1103515245, 12345, 1u << 31);
    // recupere le threadID 2d, et l'utilise directement comme coordonnees de pixel
    vec2 position = vec2(gl_GlobalInvocationID.xy);
    float rand1, rand2;
    rng = sample_rng(rng, rand1);
    rng = sample_rng(rng, rand2);
    position.x += rand1 + 0.5f;
    position.y += rand2 + 0.5f;
    
    // construction du rayon pour le pixel, passage depuis le repere image vers le repere monde
    vec4 oh = invMatrix * vec4(position, 0, 1);    // origine sur near
    vec4 eh = invMatrix * vec4(position, 1, 1);    // extremite sur far

    // origine et direction
    vec3 o = oh.xyz / oh.w;    // origine
    vec3 d = eh.xyz / eh.w - o;    // direction

    float hit = 1;    // tmax = far, une intersection valide est plus proche que l'extremite du rayon / far...
    float hit2 = 1;
    float hitu = 0;
    float hitv = 0;
    int index = -1;

    vec3 acc = vec3(0, 0, 0);
    vec3 color = vec3(0, 0, 0);
    for (int i = 0; i < triangles.length(); i++)
    {
        Triangle triangle = triangles[i];
        float t, u, v;
        if (intersect(triangle, o, d, hit, t, u, v))
        {
            if (t < hit) {
                hit = t;
                hitu = u;
                hitv = v;
                index = i;
            }
        }  
    }
    if (index >= 0) {
        Triangle triangle = triangles[index];

        vec3 pn = normalize(cross(triangle.ab, triangle.ac));
        if (dot(pn, d) > 0.0f) pn = -pn;
        vec3 p = o + hit * d + (0.001f * pn);

        vec3 t;
        vec3 b;
        world(pn, t, b);

        int N = 16;
        for (int j = 0; j < N; j++) {
            // float r1;
            float r1, r2;
            rng = sample_rng(rng, r1);
            rng = sample_rng(rng, r2);

            // vec3 direction = normalize(fibo(j, N, r1));
            vec3 direction = normalize(sample35(r1, r2));
            float pdf = pdf35(direction);
                
            if (pdf != 0) {
                vec3 vk = normalize(local_to_global(direction, t, b, pn));

                float v = 1.0f;

                for (int k = 0; k < triangles.length(); k++) {
                    float t2, u2, v2;
                    if (intersect(triangles[k], p, vk, hit2, t2, u2, v2)) {
                        v = 0.0f;
                        break;
                    }
                }

                // acc += ((triangle.diffuse / M_PI) * v * direction.z) / (float(M_PI) * pdf);
                acc += ((triangle.diffuse / M_PI) * v * max(0.0f, dot(vk, pn))) / (float(M_PI) * pdf);
            }
        }
        acc /= float(N);

        acc.x = pow(acc.x, 1.0f / 2.2f);
        acc.y = pow(acc.y, 1.0f / 2.2f);
        acc.z = pow(acc.z, 1.0f / 2.2f);

        color = acc;
    }
    
    
    // ecrire le resultat dans l'image
    imageStore(image, ivec2(gl_GlobalInvocationID.xy), vec4(color, 1));
    // imageStore(image, ivec2(gl_GlobalInvocationID.xy), vec4(hitu, hitv, 0, 1));
}
#endif