#version 120
#define TEST 0

attribute vec4 vertexPosition; 

uniform mat4 transform;
uniform mat4 projector;
uniform mat4 losMatrix;
uniform vec3 waterNormal;
uniform float waterD;
uniform float time;

uniform sampler2D heightMap1;
uniform sampler2D heightMap2;
uniform sampler2D heightMap3;

varying vec2 losCoords;
varying vec4 waterCoords;
varying float waterHeight;


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

    vec3 scale = vec3(0.01, 0.01, 0.01);
    vec2 wind1 = vec2(1, 3);
    vec2 wind2 = vec2(-3, 1);
    vec2 wind3 = vec2(0.3, -1);
    vec3 timeScale = vec3(0.01, 0.01, 0.03);
    vec3 amplitude = vec3(0.8, 1.2, 0.5);

#if TEST
	vec3 imgHeight = texture2D(heightMap1, 0.005 * intersection.xz + wind1).rgb
        * amplitude.x - 0.5;
	intersection.y += imgHeight.g;
	intersection.xz += imgHeight.rb;

	imgHeight = texture2D(heightMap2, 0.004 * intersection.xz + wind2).rgb *
        amplitude.y - 0.5;
	intersection.y += imgHeight.g;
	intersection.xz += imgHeight.rb;

	imgHeight = texture2D(heightMap3, 0.04 * intersection.xz + wind3).rgb *
        amplitude.z - 0.5;
	intersection.y += imgHeight.g;
	intersection.xz += imgHeight.rb;
#else
	vec3 imgHeight = texture2D(heightMap1, scale.x * intersection.xz + wind1 *
            timeScale.x * time).rgb * amplitude.x - 0.5;
	intersection.y += imgHeight.g;
	intersection.xz += imgHeight.rb;

	imgHeight = texture2D(heightMap2, scale.y * intersection.xz + wind2 *
            timeScale.y * time).rgb * amplitude.y - 0.5;
	intersection.y += imgHeight.g;
	intersection.xz += imgHeight.rb;

	imgHeight = texture2D(heightMap2, scale.z * intersection.xz + wind3 *
            timeScale.z * time).rgb * amplitude.z - 0.5;
	intersection.y += imgHeight.g;
	intersection.xz += imgHeight.rb;
#endif

	losCoords = (losMatrix * intersection).rg;

    waterCoords = intersection;
    waterHeight = imgHeight.y;
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
