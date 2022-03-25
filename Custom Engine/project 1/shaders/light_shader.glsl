#version 330


#ifdef VERTEX_SHADER
layout(location= 0) in vec3 position;

uniform mat4 mvp;

void main( )
{
    gl_Position = mvp * vec4(position, 1);
}
#endif

#ifdef FRAGMENT_SHADER

out vec4 fragment_color;

uniform vec3 light_color;

void main() {
    fragment_color = vec4(light_color, 1.0f);
}
#endif
