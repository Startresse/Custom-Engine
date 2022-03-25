#version 330

#ifdef VERTEX_SHADER

in vec3 position;
in vec3 position_target;
in vec3 normal;
in vec3 normal_target;
in uint material_index;

uniform mat4 mvpMatrix;
uniform mat4 modelMatrix;
uniform float t;

out vec3 n;
out vec3 p;
flat out uint ID;

void main( )
{
    vec4 inter_pos = vec4((position * (1 - t)) + (position_target * t), 1);
    vec3 inter_norm = (normal * (1 - t)) + (normal_target * t);
    gl_Position = mvpMatrix * inter_pos;
    p = vec3(modelMatrix * inter_pos);
    n = mat3(modelMatrix) * inter_norm;
    ID = material_index;
}
#endif

#ifdef FRAGMENT_SHADER

#define NBR_MATERIALS 16

in vec3 n;
in vec3 p;
flat in uint ID;

uniform vec3 source;
uniform vec3 emission;
uniform vec3 camera;
uniform float spec_coeff[NBR_MATERIALS];
uniform vec4 color_diff[NBR_MATERIALS];
uniform vec4 color_spec[NBR_MATERIALS];

out vec4 fragment_color;

const float PI= 3.14159265359;

void main( )
{
    vec3 color_d = color_diff[ID].rgb;
    vec3 color_s = color_spec[ID].rgb;
    float alpha = spec_coeff[ID];

    vec3 o = normalize(camera - p);
    vec3 l = normalize(source - p);
    vec3 h = normalize(o + l);
    vec3 nn = normalize(n);

    float cos_theta = max(0, dot(nn, l));
    float cos_theta_h = max(0, dot(nn, h));

    float k = 0.9f;

    float fr_d = (k / PI) * cos_theta;
    float fr_s = (1 - k) * ((alpha + 8) / (8 * PI)) * pow(cos_theta_h, alpha);

    vec3 color_ = (fr_s * color_s + fr_d * color_d) * emission;

    float ambiant_stength = 0.1f;
    fragment_color = vec4(color_ + ambiant_stength * color_d, 1.0f);

}
#endif