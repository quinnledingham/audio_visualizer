#version 330 core

layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 texture_coords;

uniform mat4 model;
uniform mat4 projection;

out vec2 uv;

void main(void)
{
	gl_Position = projection * model * vec4(position, 1.0f);
	uv = texture_coords;
}