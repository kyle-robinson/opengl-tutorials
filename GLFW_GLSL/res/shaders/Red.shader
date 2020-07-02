#shader vertex
#version 420 core

layout(location = 0) in vec3 aPos;

layout(std140) uniform Matrices
{
    mat4 projection;
    mat4 view;
};

uniform mat4 model;

void main()
{
    gl_Position = projection * view * model * vec4(aPos, 1.0);
    gl_PointSize = gl_Position.z;
};

#shader fragment
#version 420 core
out vec4 FragColor;
layout (depth_greater) out float gl_FragDepth;

void main()
{
    if (gl_FragCoord.x < 500)
        FragColor = vec4(1.0, 0.0, 0.0, 1.0);
    else
        FragColor = vec4(1.0, 0.5, 0.7, 1.0);

    //gl_FragDepth = gl_FragCoord.z + 0.05;
};