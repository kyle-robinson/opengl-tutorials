#shader vertex
#version 330 core

layout(location = 0) in vec4 aPos;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
    gl_Position = projection * view * model * aPos;
};

#shader fragment
#version 330 core
out vec4 FragColor;

void main()
{
    FragColor = vec4(0.8, 0.6, 0.1, 1.0);
};