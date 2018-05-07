#version 120

uniform sampler2D losMap;

uniform sampler2D height;
uniform sampler2D variationMap;

uniform sampler2D normalMap1;
uniform sampler2D normalMap2;
uniform sampler2D normalMap3;

uniform vec3 ambient;
uniform vec3 sunDir;
uniform vec3 sunColor;

varying vec2 losCoords;
varying vec4 waterCoords;
varying float waterHeight;
uniform float time;

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

//vec3 calculateNormal(vec3 a, float t_cst, sampler2D normalMap);
vec3 calculateNormal(vec3 position, sampler2D normalMap);

void main()
{
	float losMod;

    vec4 shallowColor = vec4(0.0, 0.64, 0.68, 0.5);
    vec4 deepColor = vec4(0.02, 0.05, 0.10, 1.0);

	losMod = texture2D(losMap, losCoords.st).a;
	losMod = losMod < 0.03 ? 0.0 : losMod;

	vec4 color = vec4(0.0, 0.0, 0.0 , 0.0);
    float c = 1;
    color += vec4(sunColor, 1.0);
	gl_FragColor = color * losMod;
}

// Convert the normal form the normal map to the eye space. Attenuate it by a
// factor a and attenuate the time scroll with a factor t_cst
/*
vec3 calculateNormal(vec3 a, float t_cst, sampler2D normalMap) {

    // Gram-Schmidt process to re-orthogonalize the vectors
    vec3 normal = normalize(l_normal);
    vec3 tangent = normalize(l_tangent);
    tangent = normalize(tangent - dot(tangent, normal) * normal);

    // Fetch the normal from the normal map
    vec3 binormal = cross(tangent, normal);
    vec3 bumpMapNormal = texture(normalMap, l_position.xz + t_cst *
            vec2(t)).xyz;
    bumpMapNormal = 2.0 * bumpMapNormal - vec3(1.0, 1.0, 1.0);

    // Create TBN matrix
    mat3 tbn = mat3(tangent, binormal, normal);

    // Transform the bumped normal int local space, dampen it and transform it
    // into the eye space.
    vec3 newNormal = tbn * bumpMapNormal;
    newNormal = normalize(a * newNormal);
    return vec3(V * M * vec4(newNormal, 0));
}
*/

vec3 calculateNormal(vec3 position)
{
    vec3 n = texture2D(normalMap1, scale.x * waterCoords.xy +
            wind1 * timeScale1 * time).rgb * amplitude1 - 0.5;

    n += texture2D(normalMap2, scale.x * waterCoords.xy +
            wind1 * timeScale2 * time).rgb * amplitude2 - 0.5;

    n += texture2D(normalMap3, scale.x * waterCoords.xy +
            wind1 * timeScale3 * time).rgb * amplitude3 - 0.5;

    return normalize(n);
}
