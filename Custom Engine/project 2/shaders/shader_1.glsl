#version 330

#ifdef VERTEX_SHADER
layout(location= 0) in vec3 position;
layout(location= 2) in vec3 normal;

uniform mat4 mvpMatrix;
uniform mat4 modelMatrix;
uniform mat4 viewMatrix;
uniform mat4 viewInvMatrix;

out vec3 p;
out vec3 n;

void main( )
{
    p = vec3(modelMatrix * vec4(position, 1));
    n = mat3(modelMatrix) * normal;
    gl_Position = mvpMatrix * vec4(position, 1);
}

#endif


#ifdef FRAGMENT_SHADER

in vec3 p;
in vec3 n;

uniform mat4 viewInvMatrix;
uniform vec3 pos_light;
uniform int indexMesh;

out vec4 fragment_color;

vec3 color(int id) {
    vec3 ret = vec3(0.11124f, 0.432112f, 0.6654f);
    ret = ret * (id + 1);
    ret.x = ret.x - floor(ret.x);
    ret.y = ret.y - floor(ret.y);
    ret.z = ret.z - floor(ret.z);
    return 0.8f * ret;
}

void main( )
{
    vec3 default_color = color(indexMesh);
    vec3 light_color = vec3(1.0f, 1.0f, 1.0f);

    // utility
    vec3 norm = normalize(n);
    vec3 light_direction = normalize(pos_light - p);

    // ambient
    float ambient_strength = 0.2f;
    vec3 ambient = ambient_strength * light_color * default_color;

    // diffuse
    float diffuse_strength = 1.0f;
    float dot_light_normal = dot(norm, light_direction);
    float diff = max(dot_light_normal, 0.0);
    vec3 diffuse = diffuse_strength * diff * light_color * default_color;

    // specular
    float specular_strength = 0.5f;
    vec3 viewPos = vec3(viewInvMatrix * vec4(0, 0, 0, 1));
    vec3 view_direction = normalize(viewPos - p);
    vec3 reflect_direction = reflect(-light_direction, norm);
    float spec = pow(max(dot(view_direction, reflect_direction), 0.0), 16);
    vec3 specular = specular_strength * spec * light_color;

    // normal shading
    fragment_color = vec4(
        (diffuse + ambient + specular) / (ambient_strength + diffuse_strength + specular_strength),
        1.0f
    );

    // cell shading
    // fragment_color = floor(
    //     5 * ((diffuse + ambient + specular) / (ambient_strength + 1.0 + specular_strength))
    // ) / 5;
}

#endif
