#version 430

#ifdef COMPUTE_SHADER

layout(binding = 0, std430) writeonly buffer resData
{
    uint res[];
};

uniform uint N;

layout(local_size_x= 1024) in;
void main( )
{
	uint id= gl_GlobalInvocationID.x;

    if (id < N) {
        res[id] = id;
    }

}

#endif

