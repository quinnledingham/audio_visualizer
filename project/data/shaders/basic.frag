#version 330 core

uniform vec4 my_color;
out vec4 FragColor;

void main()
{
	vec4 color = vec4(my_color.x/255, my_color.y/255, my_color.z/255, my_color.w/255);
	FragColor = color;
} 