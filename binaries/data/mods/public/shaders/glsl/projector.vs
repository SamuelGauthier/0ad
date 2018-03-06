#version 120

attribute vec2 vertexPosition; 

uniform mat4 transform;

void main()
{
	gl_Position = transform * vec4(vertexPosition, 0, 0);
}
