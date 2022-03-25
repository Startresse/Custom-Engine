#version 330


#ifdef VERTEX_SHADER
layout(location= 0) in vec3 position;

uniform mat4 mvpMatrix;

out vec3 p;

void main( )
{
    p = vec3(mvpMatrix * vec4(position, 1));
}

#endif

#ifdef FRAGMENT_SHADER

in vec3 p;

void main() {

}

#endif