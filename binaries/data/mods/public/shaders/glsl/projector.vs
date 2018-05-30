#version 120

attribute vec4 vertexPosition; 

uniform mat4 MVP;
uniform mat4 projectorMVP;
uniform mat4 losMatrix;
uniform vec3 waterNormal;
uniform float waterD;
uniform float time;
uniform mat4 reflectionMVP;
uniform mat4 refractionMVP;

uniform sampler2D heightMap1;
uniform sampler2D heightMap2;
uniform sampler2D heightMap3;
uniform sampler2D normalMap1;
uniform sampler2D normalMap2;
uniform sampler2D normalMap3;
uniform sampler2D variationMap;

varying vec3 normal;
varying vec2 losCoords;
varying vec4 waterCoords;
varying float waterHeight;
varying vec3 intersectionPos;
varying vec3 reflectionCoords;
varying vec3 refractionCoords;
varying vec3 gradient;

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
vec3 computeDisplacement(vec2 uv, float variation);
vec3 computeNormal(vec2 uv, float variation);

void main()
{
	vec4 start = vertexPosition;
	vec4 end = vertexPosition;

	start.z = 1.0;
	end.z = -1.0;
	start = projectorMVP * start;
	start /= start.w;
	end = projectorMVP * end;
	end /= end.w;

	vec4 intersection = FindLineSegIntersection(start, end);
    waterCoords = intersection;

    intersectionPos = intersection.xyz;
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
    wind2 = vec2(-3, -2);
    //wind2 = vec2(-1, -3);
    //wind3 = vec2(0, 1);
    wind3 = vec2(3, -1);
    timeScale1 = 0.006;
    timeScale2 = 0.006;
    timeScale3 = 0.005;
    //amplitude1 = 0.8;
    //amplitude2 = 1.2;
    //amplitude3 = 0.5;
    amplitude1 = 1.8;
    amplitude2 = 2.2;
    amplitude3 = 1.5;

    //float variation = texture2D(variationMap, 0.0001 * waterCoords.xz).r;
    float variation = texture2D(heightMap1, 0.0001 * waterCoords.xz).g;
    normal = computeNormal(intersection.xz, variation);
    vec3 h = computeDisplacement(intersection.xz, variation);

    //vec3 h = texture2D(heightMap1, scale.x * intersection.xz + wind1 *
    //        timeScale1 * time).rgb * amplitude1 - 0.5;

    //h += texture2D(heightMap2, scale.x * intersection.xz + wind2 *
    //        timeScale2 * time).rgb * amplitude2 - 0.5;

    //h += texture2D(heightMap3, scale.x * intersection.xz + wind3 *
    //        timeScale3 * time).rgb * amplitude3 - 0.5;

    //h *= variation;
	losCoords = (losMatrix * vec4(intersection.xyz, 1.0)).rg;
    //intersection.xyz += h;

    intersectionPos = intersection.xyz;


    reflectionCoords = (reflectionMVP * vec4(intersectionPos.xyz, 1.0)).rga;
	refractionCoords = (refractionMVP * vec4(intersectionPos.xyz, 1.0)).rga;

	gl_Position = MVP * intersection;
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

vec3 computeDisplacement(vec2 uv, float variation)
{
    //float variation = texture2D(heightMap1, 0.0001 * waterCoords.xz).g;

    //vec3 h = texture2D(heightMap1, scale.x * uv + wind1 *
    //        timeScale1 * time).rgb * amplitude1 - 0.5;
    vec3 h = 2.0 * texture2D(heightMap1, scale.x * uv).rgb - 1.0;

    //h += texture2D(heightMap2, scale.x * uv + wind2 *
    //        timeScale2 * time).rgb * amplitude2 - 0.5;

    //h += texture2D(heightMap3, scale.x * uv + wind3 *
    //        timeScale3 * time).rgb * amplitude3 - 0.5;

	return h;//* variation;
}

//vec2 sobel(vec2 uv)
//{

//}
//

vec3 computeNormal(vec2 uv, float variation)
{
    vec3 n = texture2D(normalMap1, scale.x * uv).rgb;
    //vec3 n = texture2D(normalMap1, scale.x * uv +
    //        wind1 * timeScale1 * time).rgb * amplitude1;
    //n += texture2D(normalMap2, scale.x * uv +
    //        wind2 * timeScale2 * time).rgb * amplitude2;
    //n += texture2D(normalMap3, scale.x * uv +
    //        wind3 * timeScale3 * time).rgb * amplitude3;

	vec3 normal = n;
	normal.yz = n.zy;
    return normalize(2.0 * normal - 1.0);
    //return normalize(2.0 * normal - 3.0);
}
