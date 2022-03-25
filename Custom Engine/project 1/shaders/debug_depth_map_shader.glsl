#version 330


#ifdef VERTEX_SHADER
layout(location= 0) in vec3 position;
layout(location= 1) in vec2 aTexCoords;

out vec2 TexCoords;

void main( )
{
    TexCoords = aTexCoords;
    gl_Position = vec4(position, 1.0);
}

#endif

#ifdef FRAGMENT_SHADER

out vec4 FragColor;
in vec2 TexCoords;

uniform sampler2D depthMap;

void main() {
    float depthValue = texture(depthMap, TexCoords).r;
    FragColor = vec4(vec3(depthValue), 1.0);
}

#endif
