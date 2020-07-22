#shader vertex
#version 330 core
layout(location = 0) in vec3 aPos;
layout(location = 1) in vec2 aTexCoords;

out vec3 FragPos;
out vec2 TexCoords;

uniform mat4 projection;
uniform mat4 view;

uniform float time;
uniform float speed;
uniform float amount;
uniform float height;

void main()
{
	FragPos = aPos;
	TexCoords = aTexCoords;

	float xx = (aPos.x - 3) * (aPos.x - 3);
	float yy = (aPos.y + 1) * (aPos.y + 1);
	float y = sin(time * speed + (aPos.x * aPos.z * amount) + 0.5 * cos(aPos.x * aPos.z * amount)) * height;

	gl_Position = projection * view * vec4(aPos.x, y, aPos.z, 1.0);
}

#shader fragment
#version 330 core
out vec4 FragColor;

in vec3 FragPos;
in vec2 TexCoords;

uniform sampler2D waterTexture;
uniform sampler2D lavaTexture;
uniform sampler2D emissionTexture;
uniform samplerCube skybox;
uniform vec3 cameraPos;

uniform int viewportWidth;
uniform float viewportMultiplier;
uniform bool lighting;
uniform bool isLava;
uniform bool bothTextures;

void main()
{
	vec4 waterMap = texture(waterTexture, TexCoords);
	vec4 lavaMap = texture(lavaTexture, TexCoords);
	vec4 emissionMap = texture(emissionTexture, TexCoords);

	vec4 lava = vec4(lavaMap.rgb, 1.0f) + vec4(emissionMap.rgb, 1.0f);
	FragColor = (isLava ? lava : vec4(waterMap.rgb, 0.1f));
	if (!isLava)
	{
		vec3 Normals = normalize(cross(dFdx(FragPos), dFdy(FragPos)));

		float ratio = 1.00 / 1.33;
		vec3 viewDir = normalize(FragPos - cameraPos);
		vec3 refraction = refract(viewDir, normalize(-Normals), ratio);
		FragColor *= vec4(texture(skybox, refraction).rgb, 1.0);

		vec3 reflection = reflect(viewDir, normalize(-Normals));
		FragColor *= vec4(texture(skybox, reflection).rgb, 1.0);
	}

	if (bothTextures)
	{
		if (gl_FragCoord.x < (viewportWidth * viewportMultiplier))
		{
			FragColor = vec4(waterMap.rgb, 1.0f);

			vec3 Normals = normalize(cross(dFdx(FragPos), dFdy(FragPos)));

			float ratio = 1.00 / 1.33;
			vec3 viewDir = normalize(FragPos - cameraPos);
			vec3 refraction = refract(viewDir, normalize(-Normals), ratio);
			FragColor *= vec4(texture(skybox, refraction).rgb, 1.0);

			vec3 reflection = reflect(viewDir, normalize(-Normals));
			FragColor *= vec4(texture(skybox, reflection).rgb, 1.0);
		}
		else
			FragColor = lava;
	}


	vec3 diffuse = vec3(-1.0, -1.0, -1.0);
	float attenuation = dot(-normalize(cross(dFdx(FragPos), dFdy(FragPos))), diffuse);
	attenuation = max(attenuation, 0.0);

	vec3 hazy_ambient = 0.4 * vec3(0.741, 0.745, 0.752);
	vec3 sunrise_ambient = 0.4 * vec3(0.713, 0.494, 0.356);

	FragColor.xyz *= ((lighting ? hazy_ambient : sunrise_ambient) + attenuation);
	FragColor.a = 0.9;
}