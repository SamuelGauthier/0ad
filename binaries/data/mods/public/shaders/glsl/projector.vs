#version 120

attribute vec2 a_vertex; 

uniform mat4 transform;

void main()
{
	gl_Position = transform * vec4(a_vertex, 0, 0);
}