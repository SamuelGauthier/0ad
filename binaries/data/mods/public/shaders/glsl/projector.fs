#version 120

// Schlick's R(theta=0) = ((1-1.333)/(1+1.333))^2
#define F0 0.02037318784
#define PI 3.14159265358979

uniform sampler2D losMap;

uniform sampler2D height;
uniform sampler2D variationMap;

uniform sampler2D heightMap1;
uniform sampler2D normalMap1;
uniform sampler2D normalMap2;
uniform sampler2D normalMap3;

uniform vec3 ambient;
uniform vec3 sunDir;
uniform vec3 sunColor;
uniform vec3 cameraPos;
uniform float time;

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

//vec3 calculateNormal(vec3 a, float t_cst, sampler2D normalMap);
vec3 calculateNormal(vec2 position);
vec3 calculateHeight(vec2 position);
float F(float F_0, vec3 v, vec3 h);
float G_smith(float nx, float k);
float G(float nl, float nv, float k);
float D(float nh, float alpha2);

void main()
{
    float roughness = 0.6;
    float alpha = roughness * roughness;
    float alpha2 = alpha * alpha;
    float k = alpha / 2;

	float losMod;
    //vec3 n = texture2D(heightMap1, 0.01*waterCoords.xz).rgb;
    vec3 n = calculateNormal(waterCoords.xz);

    //vec3 n = vec3(0,1,0);
    vec3 l = normalize(sunDir);
    vec3 r = reflect(l, n);
    vec3 v = normalize(cameraPos - intersectionPos);
    vec3 h = normalize(l + v);

    float nl = max(dot(n, l), 0);
    float nv = max(dot(n, v), 0);
    float nh = max(dot(n, h), 0);

    float F = F(F0, l, h);
    float G = G(nl, nv, k);
    float D = D(nh, alpha2);
    float specular = F * G * D / max(4 * nl * nv, 0.001);

    vec4 shallowColor = vec4(0.0, 0.64, 0.68, 0.5);
    vec4 deepColor = vec4(0.02, 0.05, 0.10, 1.0);

	losMod = texture2D(losMap, losCoords.st).a;
	losMod = losMod < 0.03 ? 0.0 : losMod;

	vec4 color = vec4(0.0, 0.0, 0.0 , 1.0);
    color += shallowColor;
    color += dot(l, n);
    color += pow(dot(r,v), 18);
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

vec3 calculateNormal(vec2 position) {
    vec3 n = texture2D(normalMap1, scale.x * position +
            wind1 * timeScale1 * time).rgb;// * amplitude1 - 0.5;

    n += texture2D(normalMap2, scale.x * position +
            wind1 * timeScale2 * time).rgb;// * amplitude2 - 0.5;

    n += texture2D(normalMap3, scale.x * position +
            wind1 * timeScale3 * time).rgb;// * amplitude3 - 0.5;

    return normalize(n);
}

vec3 calculateHeight(vec2 position) {
    vec3 h = texture2D(heightMap1, scale.x * intersection.xz + wind1 *
            timeScale1 * time).rgb * amplitude1 - 0.5;

    h += texture2D(heightMap2, scale.x * intersection.xz + wind1 *
            timeScale2 * time).rgb * amplitude2 - 0.5;

    h += texture2D(heightMap3, scale.x * intersection.xz + wind1 *
            timeScale3 * time).rgb * amplitude3 - 0.5;

    return normalize(h);
}

// Shlick approximation
float F(float F_0, vec3 v, vec3 h) {

    return F_0 + (1 - F_0) * pow((1 - max(dot(v, h), 0)), 5);
}

float G_smith(float nx, float k) {

    return nx / (nx * (1 - k) + k);
}

// Height correlated Smith
float G(float nl, float nv, float k) {

    return G_smith(nl, k) * G_smith(nv, k);
}

// Trowbridge-Reitz
float D(float nh, float alpha2) {

    float denom = nh * nh * (alpha2 - 1) + 1;
    return alpha2 / (PI * denom * denom);
}
