#shader vertex
#version 330 core

layout(location = 0) in vec2 aPos;
layout(location = 1) in vec2 aTexCoords;

out vec2 TexCoords;

void main()
{
    TexCoords = aTexCoords;
    gl_Position = vec4(aPos.x, aPos.y, 0.0, 1.0);
};

#shader fragment
#version 330 core
out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D screenTexture;

uniform bool kernelBool;
uniform bool grayscaleBool;
uniform bool nightVision;
uniform bool inverseBool;

const float offset = 1.0 / 300.0;

void main()
{
    // Kernel
    if (kernelBool)
    {
        vec2 offsets[9] = vec2[](
            vec2(-offset, offset),
            vec2(0.0f, offset),
            vec2(offset, offset),
            vec2(-offset, 0.0f),
            vec2(0.0f, 0.0f),
            vec2(offset, 0.0f),
            vec2(-offset, -offset),
            vec2(0.0f, -offset),
            vec2(offset, -offset)
        );

        // Sharpen
        float kernel[9] = float[](
            -1, -1, -1,
            -1,  9, -1,
            -1, -1, -1
        );

        // Blur
        /*float kernel[9] = float[](
            1.0 / 16,   2.0 / 16,   1.0 / 16,
            2.0 / 16,   4.0 / 16,   2.0 / 16,
            1.0 / 16,   2.0 / 16,   1.0 / 16
        );

        // Edge-Detection
        float kernel[9] = float[](
            1,  1,  1,
            1, -8,  1,
            1,  1,  1
        );*/

        vec3 sampleTex[9];
        for (int i = 0; i < 9; i++)
        {
            sampleTex[i] = vec3(texture(screenTexture, TexCoords.st + offsets[i]));
        }
        vec3 col = vec3(0.0);
        for (int i = 0; i < 9; i++)
            col += sampleTex[i] * kernel[i];

        FragColor = vec4(col, 1.0);
    }
    
    // Grayscale
    if (grayscaleBool)
    {
        FragColor = texture(screenTexture, TexCoords);
        float average = 0.2126 * FragColor.r + 0.7152 * FragColor.g + 0.0722 * FragColor.b;
        if (nightVision)
            FragColor = vec4(average, average * 3.0, average, 1.0); // Night Vision
        else    
            FragColor = vec4(average, average, average, 1.0);
    }

    // Inverse
    if (inverseBool)
        FragColor = vec4(vec3(1.0 - texture(screenTexture, TexCoords)), 1.0);

    if (!kernelBool && !grayscaleBool && !inverseBool)
        FragColor = texture(screenTexture, TexCoords);
};