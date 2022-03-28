#version 330 core

#ifdef VERTEX_SHADER

layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;

out vec3 n;
out vec3 p;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
    p = vec3(model * vec4(position, 1.0));
    n = mat3(model) * normal;
    gl_Position = projection * view * model * vec4(position, 1.0);
}
#endif


#ifdef FRAGMENT_SHADER

in vec3 n;
in vec3 p;

out vec4 frag_color;

vec3 light_color = vec3(0.8);
vec3 object_color = vec3(0.8);

uniform vec3 light_pos;

void main()
{
    float ambient_strength = 0.3f;
    vec3 ambient = ambient_strength * light_color;

    vec3 norm = normalize(n);
    vec3 light_dir = normalize(light_pos - p);

    float diff = max(dot(n, light_dir), 0.0);
    vec3 diffuse = diff * light_color;

    vec3 result = (ambient_strength * light_color + (1.f - ambient_strength) * diffuse) * object_color;
    frag_color = vec4(result, 1.0);
}
#endif
