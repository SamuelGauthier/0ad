#version 120

attribute vec4 vertexPosition; 

uniform mat4 transform;
uniform mat4 projector;
uniform vec3 waterNormal;
uniform float waterD;


vec4 FindLineSegIntersection(vec4 start, vec4 end);
float DistanceToPlane(vec4 point);

void main()
{
	//vec4 t = transform * vec4(vertexPosition, 1);
	vec4 start = vertexPosition;
	vec4 end = vertexPosition;

	start.z = 1.0;
	end.z = -1.0;
	start = projector * start;
	start /= start.w;
	end = projector * end;
	end /= end.w;

	vec4 intersection = FindLineSegIntersection(start, end);

	//vec4 l = transform * vertexPosition;
	//gl_Position = vec4(vertexPosition, 1);
	//gl_Position = l;
	gl_Position = transform * intersection;
}


vec4 FindLineSegIntersection(vec4 start, vec4 end)
{
	float dist1 = DistanceToPlane(start);
	float dist2 = DistanceToPlane(end);

	float t = (-dist1) / (dist2-dist1);

	return mix(start, end, t);
}

float DistanceToPlane(vec4 p)
{
	//waterNormal * p + waterD;
	return waterNormal.x * p.x + waterNormal.y * p.y + waterNormal.z * p.z + waterD;
}