#version 330 core

layout (location = 0) in vec3 position;
layout (location = 1) in vec4 c;

uniform mat4 MVP;
uniform int window_width;
uniform int window_height;

out vec4 color;

void main()
{
    color = c;
    gl_Position = MVP * vec4(position, 1.0);

    // put in corner with front rendering rotation
    int pixel_offset = 60;
    float x_percent = ((window_width / 2.0) - pixel_offset) / (window_width / 2.0);
    float y_percent = ((window_height / 2.0) - pixel_offset) / (window_height / 2.0);
    gl_Position.xy += vec2(x_percent, y_percent) * gl_Position.w;
}