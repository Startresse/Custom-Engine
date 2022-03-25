#version 330


#ifdef VERTEX_SHADER
layout(location= 0) in vec3 position;

uniform mat4 model;
uniform mat4 light_space_matrix;

void main( )
{
    gl_Position = light_space_matrix * model * vec4(position, 1);
}

#endif

#ifdef FRAGMENT_SHADER

void main() {

}

#endif
