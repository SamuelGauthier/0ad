#version 120

attribute vec3 vertexPosition; 

uniform mat4 transform;

void main()
{
	gl_Position = vec4(vertexPosition, 1);
}
