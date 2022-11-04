#version 430 core

layout (location = 0) in vec3 aPos;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform mat4 hipsMatrix;
uniform mat4 spineMatrix;
uniform mat4 headMatrix;
//uniform mat4 headtransform;
uniform mat4 eye_angle;
//uniform mat4 scaleonly;
//uniform vec3 offset;
void main()
{
	gl_Position = projection * view * model * hipsMatrix * spineMatrix* headMatrix * eye_angle  * vec4(aPos, 1.0);
	//gl_Position = projection * view * model * headtransform * eye_angle *scaleonly* vec4(aPos, 1.0) ;
}