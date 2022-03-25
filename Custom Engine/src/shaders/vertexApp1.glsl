#version 330 core

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