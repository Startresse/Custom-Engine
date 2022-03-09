#version 330 core

layout (location = 0) in vec3 position;

uniform mat4 MVP;

flat out int line_nb;

void main()
{

    // determine x, y or z axis
    if (position.x > 0.0f)
        line_nb = 0;
    else if (position.y > 0.0f)
        line_nb = 1;
    else
        line_nb = 2;

    gl_Position = MVP * vec4(position, 1.0);
}