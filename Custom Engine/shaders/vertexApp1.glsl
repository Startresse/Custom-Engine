#version 330 core

layout (location = 0) in vec3 p;
layout (location = 1) in vec3 n;

out vec3 normal;;

uniform mat4 transform;

void main()
{
    gl_Position = transform * vec4(p, 1.0);
    normal = n;
}