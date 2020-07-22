#shader vertex
#version 330 core
layout(location = 0) in vec3 aPos;

out vec3 TexCoords;

uniform mat4 projection;
uniform mat4 view;

void main()
{
	TexCoords = aPos;
	vec4 pos = projection * view * vec4(aPos, 1.0);
	gl_Position = pos.xyww;
}

#shader fragment
#version 330 core
out vec4 FragColor;
in vec3 TexCoords;

uniform samplerCube skybox;
uniform samplerCube hellbox;
uniform int viewportWidth;
uniform float viewportMultiplier;

uniform bool isLava;
uniform bool bothTextures;

void main()
{	
	if (bothTextures)
	{
		if (gl_FragCoord.x < (viewportWidth * viewportMultiplier))
			FragColor = texture(skybox, TexCoords);
		else
			FragColor = texture(hellbox, TexCoords);
	}
	else
	{
		if (!isLava)
			FragColor = texture(skybox, TexCoords);
		else
			FragColor = texture(hellbox, TexCoords);
	}
}