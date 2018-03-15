#version 120

attribute vec3 vertexPosition; 

uniform mat4 transform;

void main()
{
	vec4 t = transform * vec4(vertexPosition, 1);
	gl_Position = vec4(vertexPosition, 1);
}
