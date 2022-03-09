#version 330 core

flat in int line_nb;

out vec4 frag_color;

void main()
{
    frag_color = 0.8f * vec4(float(line_nb == 0), float(line_nb == 1), float(line_nb == 2), 1.0f);
} 