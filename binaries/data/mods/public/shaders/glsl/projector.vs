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

    vec3 scale = vec3(0.01, 0.02, 0.03);
    vec2 wind1 = vec2(1, 3);
    vec2 wind2 = vec2(-2, 1);
    vec2 wind3 = vec2(0.3, -1);
    vec3 timeScale = vec3(0.01, 0.01, 0.06);
    vec3 amplitude = vec3(0.8, 0.4, 0.2);
    vec3 amplitude2= 0.5 + 0.5 * vec3(sin(time * 0.01), sin(time * 0.03),
            sin(time * 0.06));

	//vec3 imgHeight = texture2D(heightMap1, 0.001 * intersection.xz + vec2(0.005, 0.005) * time).rgb;
    // TODO: Precompute the sine
	vec3 imgHeight = texture2D(heightMap1, scale.x * intersection.xz + wind1 *
            timeScale.x * time).rgb * amplitude.x * amplitude2.x;
	intersection.y += imgHeight.g;
	intersection.xz += imgHeight.rb;

	imgHeight = texture2D(heightMap2, scale.y * intersection.xz + wind2 *
            timeScale.y * time).rgb * amplitude.y * amplitude2.y;
	intersection.y += imgHeight.g;
	intersection.xz += imgHeight.rb;

	imgHeight = texture2D(heightMap2, scale.z * intersection.xz + wind3 *
            timeScale.z * time).rgb * amplitude.z * amplitude2.z;
	intersection.y += imgHeight.g;
	intersection.xz += imgHeight.rb;

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
