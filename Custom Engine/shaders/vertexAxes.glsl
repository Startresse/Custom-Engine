#version 330 core

layout (location = 0) in vec3 position;
layout (location = 1) in vec3 c;

uniform mat4 MVP;

out vec3 color;

void main()
{
    color = c;
    gl_Position = MVP * vec4(position, 1.0);
}