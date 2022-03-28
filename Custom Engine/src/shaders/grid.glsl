#version 330 core

#ifdef VERTEX_SHADER

layout (location = 0) in vec3 position;
layout (location = 1) in vec4 c;

uniform mat4 MVP;

out vec4 color;

void main()
{
    color = c;
    gl_Position = MVP * vec4(position, 1.0);
}
#endif


#ifdef FRAGMENT_SHADER

in vec4 color;

out vec4 frag_color;

void main()
{
    frag_color = vec4(color);
} 
#endif
