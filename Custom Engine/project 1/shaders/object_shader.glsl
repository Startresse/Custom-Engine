#version 330


#ifdef VERTEX_SHADER
layout(location= 0) in vec3 position;
layout(location= 2) in vec3 normal;

uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;
uniform mat4 light_space_matrix;

out vec3 p;
out vec3 n;
out vec4 position_light_space;

void main( )
{
    p = vec3(model * vec4(position, 1));
    n = mat3(model) * normal;

    position_light_space = light_space_matrix * vec4(p, 1.0);

    gl_Position = projection * view * model * vec4(position, 1);
}
#endif

#ifdef FRAGMENT_SHADER
in vec3 p;
in vec3 n;
in vec4 position_light_space;

uniform vec3 light_pos;
uniform vec3 light_color;
uniform vec3 object_color;
uniform mat4 view;

uniform sampler2D shadowMap;

out vec4 fragment_color;

float shadowCalc(float dotLightNormal) {
    vec3 pos = position_light_space.xyz * 0.5 + 0.5;
    if (pos.z > 1.0) pos.z = 1.0;
    float depth = texture(shadowMap, pos.xy).r;
    float bias = max(0.005 * (1.0 - dotLightNormal), 0.0005);
    return (depth + bias) < pos.z ? 0.0: 1.0;
}

void main() {
    vec3 viewPos = vec3(inverse(view) * vec4(0, 0, 0, 1));

    // ambient
    float ambientStrengh = .3f;
    vec3 ambient = ambientStrengh * light_color;

    // diffuse
    vec3 norm = normalize(n);
    vec3 lightDir = normalize(light_pos - p);
    float dotLightNormal = dot(norm, lightDir);
    float diff = max(dotLightNormal, 0.0);
    vec3 diffuse = diff * light_color;

    // specular
    float specularStrength = 0.5f;
    vec3 viewDir = normalize(viewPos - p);
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
    vec3 specular = specularStrength * spec * light_color;
    
    //shadow
    float shadow = shadowCalc(dotLightNormal);
    vec3 result = (ambient + (diffuse + specular) * shadow) * object_color /
                  (ambientStrengh + 1.0 + specularStrength);
    fragment_color = vec4(result, 1.0f);
}

#endif
