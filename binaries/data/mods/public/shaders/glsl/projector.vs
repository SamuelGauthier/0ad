#version 120

attribute vec4 vertexPosition; 

uniform mat4 MVP;
uniform mat4 V;
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
varying vec3 tangent;
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

varying vec3 positionCS;

vec4 FindLinePlaneIntersection(vec4 start, vec4 end);
float DistanceToPlane(vec4 point);
vec3 computeDisplacement(vec2 uv, float variation);
vec3 computeNormal(vec2 uv, float variation);
vec3 computeTangent(float y_0, vec2 x0z0);
float f_x(vec2 x0z0);
float f_z(vec2 x0z0);

float variation;

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

	vec4 intersection = FindLinePlaneIntersection(start, end);
    waterCoords = intersection;

    intersectionPos = intersection.xyz;

    scale = vec3(0.01, 0.01, 0.01);
    wind1 = vec2(1, 3);
    wind2 = vec2(-3, -2);//vec2(-1, -3);
    wind3 = vec2(3, -1);//vec2(0, 1);
    timeScale1 = 0.006;//0.8
    timeScale2 = 0.006;//1.2
    timeScale3 = 0.005;//0.5
    amplitude1 = 1.8;
    amplitude2 = 2.2;
    amplitude3 = 1.5;

    //float variation = texture2D(variationMap, 0.0001 * waterCoords.xz).r;
    variation = texture2D(heightMap1, 0.0001 * waterCoords.xz).g;
    normal = computeNormal(intersection.xz, variation);
    vec3 h = computeDisplacement(intersection.xz, variation);
    tangent = vec3(1, 0, 0);//computeTangent(h.y, intersection.xz);

	losCoords = (losMatrix * vec4(intersection.xyz, 1.0)).rg;
    intersection.xyz += h;

    intersectionPos = intersection.xyz;

    positionCS = vec3(V * intersection);

    reflectionCoords = (reflectionMVP * vec4(intersectionPos.xyz, 1.0)).rga;
	refractionCoords = (refractionMVP * vec4(intersectionPos.xyz, 1.0)).rga;

	gl_Position = MVP * intersection;
}


vec4 FindLinePlaneIntersection(vec4 start, vec4 end)
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


// Compute the displacement vector
vec3 computeDisplacement(vec2 uv, float variation)
{
    //float variation = texture2D(heightMap1, 0.0001 * waterCoords.xz).g;
    float t = time;
    //t = 1;

    vec3 h = texture2D(heightMap1, scale.x * uv + wind1 *
            timeScale1 * t).rgb * amplitude1 - 0.5;
    //vec3 h = 2.0 * texture2D(heightMap1, scale.x * uv).rgb - 1.0;

    h += texture2D(heightMap2, scale.x * uv + wind2 *
            timeScale2 * t).rgb * amplitude2 - 0.5;

    h += texture2D(heightMap3, scale.x * uv + wind3 *
            timeScale3 * t).rgb * amplitude3 - 0.5;

	return h* variation;
}

vec3 computeNormal(vec2 uv, float variation)
{
    float t = time;
    //t = 1;
    //vec3 n = texture2D(normalMap1, scale.x * uv).rgb;
    vec3 n = texture2D(normalMap1, scale.x * uv +
            wind1 * timeScale1 * t).rgb;// * amplitude1;
    n += texture2D(normalMap2, scale.x * uv +
            wind2 * timeScale2 * t).rgb;// * amplitude2;
    n += texture2D(normalMap3, scale.x * uv +
            wind3 * timeScale3 * t).rgb;// * amplitude3;

	//vec3 normal = n;
	//normal.yz = n.zy;
    //return normalize(2.0 * normal - 1.0);
    return normalize(2.0 * n - 3.0);
}

// Calculate the tangent with the sobel filter
vec3 computeTangent(float y_0, vec2 x0z0)
{
    float L00 = y_0 - f_x(x0z0)*x0z0.x - f_z(x0z0)*x0z0.y;
    float L10 = y_0 + f_z(x0z0)*(1 - x0z0.x) + f_z(x0z0)*(1 - x0z0.y);

    vec3 T0 = vec3(0, L00, 0);
    vec3 T1 = vec3(1, L10, 0);

    return normalize(T1-T0);
}

// +--+--+--+
// |a1|a4|a7|
// +--+--+--+
// |a2|a5|a8| a5 is the current center
// +--+--+--+
// |a3|a6|a9|
// +--+--+--+

float f_x(vec2 x0z0)
{
    vec2 s = vec2(1, 1)/2048;
    float a1 = computeDisplacement(x0z0 + s*vec2(-1,1), variation).y;
    float a2 = computeDisplacement(x0z0 + s*vec2(-1,0), variation).y;
    float a3 = computeDisplacement(x0z0 + s*vec2(-1,-1), variation).y;

    float a7 = computeDisplacement(x0z0 + s*vec2(1,1), variation).y;
    float a8 = computeDisplacement(x0z0 + s*vec2(1,0), variation).y;
    float a9 = computeDisplacement(x0z0 + s*vec2(1,-1), variation).y;

    return a1 + 2*a2 + a3 - (a7 + 2*a8 + a9);
}

float f_z(vec2 x0z0)
{
    vec2 s = vec2(1, 1)/2048;
    float a1 = computeDisplacement(x0z0 + s*vec2(-1,1), variation).y;
    float a4 = computeDisplacement(x0z0 + s*vec2(0,1), variation).y;
    float a7 = computeDisplacement(x0z0 + s*vec2(1,1), variation).y;

    float a3 = computeDisplacement(x0z0 + s*vec2(-1,-1), variation).y;
    float a6 = computeDisplacement(x0z0 + s*vec2(0,-1), variation).y;
    float a9 = computeDisplacement(x0z0 + s*vec2(1,-1), variation).y;

    return a1 + 2*a4 + a7 - (a3+ 2*a6 + a9);
}
