#version 330 core

in vec3 n;
in vec3 p;

out vec4 frag_color;

vec3 light_color = vec3(0.8, 0.5, 0.3);
vec3 object_color = vec3(1.0, 1.0, 1.0);
vec3 light_pos = vec3(0.0, 0.0, 30.0);

void main()
{
    float ambient_strength = 0.1f;
    vec3 ambient = ambient_strength * light_color;

    vec3 norm = normalize(n);
    vec3 light_dir = normalize(light_pos - p);

    float diff = max(dot(n, light_dir), 0.0);
    vec3 diffuse = diff * light_color;

    vec3 result = (ambient + diffuse) * object_color;
    frag_color = vec4(result, 1.0);
}
