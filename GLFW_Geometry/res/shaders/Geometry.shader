#shader vertex
#version 330 core
layout(location = 0) in vec2 aPos;
layout(location = 1) in vec3 aColor;

out VS_OUT
{
	vec3 color;
} vs_out;

uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;

void main()
{
	vs_out.color = aColor;
	gl_Position = projection * view * model * vec4(aPos, 0.0, 1.0);
};

#shader fragment
#version 330 core
out vec4 FragColor;

in vec3 fColor;

void main()
{
	FragColor = vec4(fColor, 1.0);
};

#shader geometry
#version 330 core
layout(points) in;
layout(triangle_strip, max_vertices = 5) out;

in VS_OUT
{
	vec3 color;
} gs_in[];

out vec3 fColor;

void build_house(vec4 position)
{
	fColor = gs_in[0].color;
	gl_Position = position + vec4(-0.2, -0.2, 0.0, 0.0);
	EmitVertex();
	gl_Position = position + vec4(0.2, -0.2, 0.0, 0.0);
	EmitVertex();
	gl_Position = position + vec4(-0.2, 0.2, 0.0, 0.0);
	EmitVertex();
	gl_Position = position + vec4(0.2, 0.2, 0.0, 0.0);
	EmitVertex();
	gl_Position = position + vec4(0.0, 0.4, 0.0, 0.0);
	fColor = vec3(1.0, 1.0, 1.0);
	EmitVertex();
	EndPrimitive();
}

void main()
{
	build_house(gl_in[0].gl_Position);
};