#version 120

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
uniform sampler2D variationMap;

varying vec2 losCoords;
varying vec4 waterCoords;
varying float waterHeight;
varying vec3 intersectionPos;

// Properties
varying vec3 scale;
varying vec2 wind1;
varying vec2 wind2;
varying vec2 wind3;
varying float timeScale1;
varying float timeScale2;
varying float timeScale3;
varying float amplitude1;
varying float amplitude2;
varying float amplitude3;

vec4 FindLineSegIntersection(vec4 start, vec4 end);
float DistanceToPlane(vec4 point);

void main()
{
	vec4 start = vertexPosition;
	vec4 end = vertexPosition;

	start.z = 1.0;
	end.z = -1.0;
	start = projector * start;
	start /= start.w;
	end = projector * end;
	end /= end.w;

	vec4 intersection = FindLineSegIntersection(start, end);
    waterCoords = intersection;

    /*
    vec3 scale = vec3(0.01, 0.01, 0.01);
    vec2 wind1 = vec2(1, 3);
    vec2 wind2 = vec2(-3, 1);
    vec2 wind3 = vec2(0, 1);
    float timeScale1 = 0.006;
    float timeScale2 = 0.006;
    float timeScale3 = 0.005;
    float amplitude1 = 0.8;
    float amplitude2 = 1.2;
    float amplitude3 = 0.5;
    vec3 amplitude = vec3(0.8, 1.2, 0.5);
    */
    scale = vec3(0.01, 0.01, 0.01);
    wind1 = vec2(1, 3);
    //wind2 = vec2(-3, 1);
    wind2 = vec2(-1, -3);
    wind3 = vec2(0, 1);
    timeScale1 = 0.006;
    timeScale2 = 0.006;
    timeScale3 = 0.005;
    //amplitude1 = 0.8;
    //amplitude2 = 1.2;
    //amplitude3 = 0.5;
    amplitude1 = 1.8;
    amplitude2 = 2.2;
    amplitude3 = 1.5;

    float variation = texture2D(variationMap, 0.0001 * waterCoords.xz).r;

    vec3 h = texture2D(heightMap1, scale.x * intersection.xz + wind1 *
            timeScale1 * time).rgb * amplitude1 - 0.5;

    h += texture2D(heightMap2, scale.x * intersection.xz + wind1 *
            timeScale2 * time).rgb * amplitude2 - 0.5;

    h += texture2D(heightMap3, scale.x * intersection.xz + wind1 *
            timeScale3 * time).rgb * amplitude3 - 0.5;

    h *= variation;
    intersection.xyz += h;

    intersectionPos = intersection.xyz;

	losCoords = (losMatrix * intersection).rg;

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
