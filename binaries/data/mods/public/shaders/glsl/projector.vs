#version 120

attribute vec4 vertexPosition; 

uniform mat4 transform;
uniform mat4 projector;
uniform mat4 losMatrix;
uniform vec3 waterNormal;
uniform float waterD;
uniform float time;

uniform sampler2D height;

varying vec2 losCoords;


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
	vec3 img_height = texture2D(height, intersection.xz + vec2(0.1, 0.1) * time).rgb;
	float d_h = dot(img_height, vec3(0.299, 0.587, 0.114));
	//float d_h = texture2D(height, intersection.xz + vec2(0.1, 0.1) * time).r;
	//float d_x = texture2D(height, intersection.xz + vec2(0.1, 0.1) * time).g;
	//float d_y = texture2D(height, intersection.xz + vec2(0.1, 0.1) * time).b;
    //intersection.y += sin(5 * time + intersection.x + intersection.z);
	//intersection.x += d_x;
    //intersection.y += d_h;
	//intersection.z += d_z;
	intersection.y += d_h;

	losCoords = (losMatrix * intersection).rg;

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
