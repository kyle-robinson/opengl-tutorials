#shader vertex
#version 330 core

layout(location = 0) in vec3 aPos;
layout(location = 1) in vec2 aTexCoords;

layout(std140) uniform Matrices
{
    mat4 projection;
    mat4 view;
};

out VS_OUT
{
    vec2 TexCoords;
} vs_out;

uniform mat4 model;

void main()
{
    vs_out.TexCoords = aTexCoords;
    gl_Position = projection * view * model * vec4(aPos, 1.0);
};

#shader fragment
#version 330 core
out vec4 FragColor;

in VS_OUT
{
    vec2 TexCoords;
} fs_in;

uniform sampler2D frontTexture;
uniform sampler2D backTexture;

void main()
{
    if (gl_FrontFacing)
        FragColor = texture(frontTexture, fs_in.TexCoords);
    else
        FragColor = texture(backTexture, fs_in.TexCoords);

    //FragColor = vec4(0.0, 1.0, 0.0, 1.0);
};