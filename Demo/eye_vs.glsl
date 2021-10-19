#version 430 core

layout (location = 0) in vec3 aPos;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform mat4 headMatrix;

void main()
{
	gl_Position = projection * view * model * headMatrix*vec4(aPos, 1.0);
}