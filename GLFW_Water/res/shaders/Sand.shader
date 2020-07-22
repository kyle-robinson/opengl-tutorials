#shader vertex
#version 330 core
layout(location = 0) in vec3 aPos;
layout(location = 1) in vec2 aTexCoords;

out vec3 FragPos;
out vec2 TexCoords;

uniform mat4 projection;
uniform mat4 view;

void main()
{
	FragPos = aPos;
	TexCoords = aTexCoords;
	gl_Position = projection * view * vec4(aPos, 1.0);
}

#shader fragment
#version 330 core
out vec4 FragColor;

in vec3 FragPos;
in vec2 TexCoords;

uniform sampler2D sandTexture;
uniform int viewportWidth;
uniform float viewportMultiplier;

uniform bool lighting;
uniform bool isLava;
uniform bool bothTextures;

void main()
{
	FragColor = texture(sandTexture, TexCoords);

	vec4 water1 = vec4(0.2, 0.1, 0.0, 1.0);
	vec4 water3 = vec4(0.0, 0.4, 0.2, 1.0);
	vec4 water2 = mix(water1, water3, 0.5);

	vec4 lava1 = vec4(0.05, 0.05, 0.05, 1.0);
	vec4 lava3 = vec4(0.2, 0.05, 0.05, 1.0);
	vec4 lava2 = mix(lava1, lava3, 0.5);

	if (!bothTextures)
	{
		if (FragPos.y <= 1.5)
			FragColor *= (isLava ? lava1 : water1);
		else if (FragPos.y > 1.5 && FragPos.y < 6)
			FragColor *= (isLava ? lava2 : water2);
		else if (FragPos.y > 6)
			FragColor *= (isLava ? lava3 : water3);
	}
	else
	{
		if (gl_FragCoord.x < (viewportWidth * viewportMultiplier))
		{
			if (FragPos.y <= 1.5)
				FragColor *= water1;
			else if (FragPos.y > 1.5 && FragPos.y < 6)
				FragColor *= water2;
			else if (FragPos.y > 6)
				FragColor *= water3;
		}
		else
		{
			if (FragPos.y <= 1.5)
				FragColor *= lava1;
			else if (FragPos.y > 1.5 && FragPos.y < 6)
				FragColor *= lava2;
			else if (FragPos.y > 6)
				FragColor *= lava3;
		}
	}

	vec3 diffuse = vec3(-1.0);
	float attenuation = dot(-normalize(cross(dFdx(FragPos), dFdy(FragPos))), diffuse);
	attenuation = max(attenuation, 0.0);

	vec3 hazy_ambient = 0.4 * vec3(0.741, 0.745, 0.752);
	vec3 sunrise_ambient = 0.2 * vec3(0.712, 0.494, 0.356);

	FragColor.xyz *= ((lighting ? hazy_ambient : sunrise_ambient) + attenuation);
	FragColor.a = 1.0;
}