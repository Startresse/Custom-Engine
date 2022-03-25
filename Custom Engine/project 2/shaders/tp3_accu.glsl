#version 430

#ifdef COMPUTE_SHADER

/* UNIFORMS */

uniform uint frame;
layout(binding = 0, rgba16) coherent uniform image2DArray image;
layout(binding = 3, rgba16) readonly uniform image2DArray color_image;


/* UTILITY */

vec3 gamma(vec3 color, bool inv)
{
    float coeff = 2.2;
    if (!inv)
        coeff = 1.0/coeff;
    
    return pow(color, vec3(coeff));
}


/* MAIN */

layout( local_size_x= 8, local_size_y= 8 ) in;
void main( )
{
    ivec2 xy = ivec2(gl_GlobalInvocationID.xy);

    vec3 pixel_color = vec3(0.0);
    for (int i = 0; i < 4; ++i)
    {
        vec3 pred_color = imageLoad(image, ivec3(xy, i)).xyz;
        vec3 color = imageLoad(color_image, ivec3(xy, i)).xyz;

        color = (pred_color * frame + color) / (frame + 1);
        pixel_color += color;

        imageStore(image, ivec3(xy, i), vec4(color, 1));
    }

    // apply gamma correction and display
    pixel_color = gamma(pixel_color, false);
    imageStore(image, ivec3(xy, 4), vec4(pixel_color, 1));
}

#endif

