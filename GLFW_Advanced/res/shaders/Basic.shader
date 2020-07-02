#shader vertex
#version 330 core

layout(location = 0) in vec4 aPos;
layout(location = 1) in vec2 aTexCoords;

out vec2 TexCoords;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
    TexCoords = aTexCoords;
    gl_Position = projection * view * model * aPos;
};

#shader fragment
#version 330 core
out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D texture1;

float near = 0.1;
float far = 100.0;

float LinearizeDepth(float depth)
{
    float z = depth * 2.0 - 1.0;
    return (2.0 * near * far) / (far + near - z * (far - near));
}

void main()
{
    // Depth Testing
    //float depth = LinearizeDepth(gl_FragCoord.z) / far;
    //FragColor = vec4(vec3(depth), 1.0);

    // Transparency
    vec4 texColor = texture(texture1, TexCoords);
    if (texColor.a < 0.1)
        discard;
    FragColor = texColor;
};