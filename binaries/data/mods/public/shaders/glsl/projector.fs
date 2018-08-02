#version 120

// Schlick's R(theta=0) = ((1-1.333)/(1+1.333))^2
#define F0 0.02037318784
#define PI 3.14159265358979

// air/water = 1/1.333
//#define RATIO 0.7501875469
#define RATIO 0.909

uniform sampler2D losMap;

uniform sampler2D height;
uniform sampler2D variationMap;

uniform sampler2D heightMap1;
uniform sampler2D heightMap2;
uniform sampler2D heightMap3;
uniform sampler2D normalMap1;
uniform sampler2D normalMap2;
uniform sampler2D normalMap3;

uniform sampler2D reflectionMap;
uniform vec3 reflectionFarClipN;
uniform float reflectionFarClipD;
uniform mat4 reflectionMVP; 

uniform sampler2D refractionMap;
uniform sampler2D refractionMapDepth;
uniform vec3 refractionFarClipN;
uniform float refractionFarClipD;
uniform mat4 refractionMVP; 

uniform sampler2D entireSceneDepth;
uniform vec3 farClipN;
uniform float farClipD;
uniform mat4 fullMVP;
uniform mat4 invFullMVP;

uniform sampler2D terrain;
uniform float terrainWorldSize;
uniform float terrainHeightScale;
uniform float maxTerrainElevation;
uniform float minTerrainElevation;
uniform float waterH;

uniform mat4 P;
uniform mat4 V;
uniform mat4 MVP;
uniform mat4 invP;
uniform mat4 invV;
uniform mat4 invMVP;
uniform mat4 projectorMVP;
uniform mat4 skyBoxRot;

uniform float screenWidth;
uniform float screenHeight;
uniform float nearPlane;
uniform float farPlane;
uniform vec4 unproject;
uniform vec2 viewport;
uniform vec2 projectionAB;

uniform vec3 ambient;
uniform vec3 sunDir;
uniform vec3 sunColor;
uniform vec3 cameraPos;
uniform float time;

uniform samplerCube skyCube;

uniform vec3 screenCenter;

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

vec3 CalculateNormal(vec2 uv);
vec3 CalculateHeight(vec2 uv, float variation);
float F(float F_0, vec3 v, vec3 h);
float G_smith(float nx, float k);
float G(float nl, float nv, float k);
float D(float nh, float alpha2);
vec4 ComputeReflection(vec3 n);
vec3 ComputeRefraction(vec3 n, vec3 v);
float Sum(vec4 t);
vec3 LightAttenuation(vec3 color, float depth);
vec3 GetWorldPosFromDepth();
vec3 LineLineIntersection(vec3 o1, vec3 d1, vec3 o2, vec3 d2);
float determinant(mat3 m);
vec3 GetWorldFromHeightMap(vec2 worldXZ);
vec3 GetPosOnTerrain(vec3 p);

float waterDepth;

void main()
{
    float roughness = 0.3;
    float alpha = roughness * roughness;
    float alpha2 = alpha * alpha;
    float k = alpha / 2;

	float losMod;
    float variation = texture2D(variationMap, 0.001 * waterCoords.xz).r;
	vec2 variationWind = vec2(-1,-1);
	float variationTS = 0.003;
    //float variation = texture2D(heightMap1, 0.001 * waterCoords.xz + variationWind * variationTS * time).g;
    //vec3 normalAttenuation = vec3(0.1, 1, 0.1);
	vec3 n = normalize(normal);
	//n.yz = n.zy;
    n = CalculateNormal(waterCoords.xz);
    //n = vec3(0, 1, 0);
    //n = texture2D(heightMap1, 0.01*waterCoords.xz).rgb;

    vec3 l = sunDir;
    //l = normalize(reflect(sunDir, n));
    vec3 r = reflect(l, n);
	vec3 p = cameraPos;
	p.yz = cameraPos.zy;
    vec3 v = normalize(cameraPos - intersectionPos);
    vec3 h = normalize(l + v);

    float nl = max(dot(n, l), 0);
    float nv = max(dot(n, v), 0);
    float nh = max(dot(n, h), 0);

    float F_val = F(F0, l, h);
    float G = G(nl, nv, k);
    float D = D(nh, alpha2);
    float specular = F_val * G * D / max(4 * nl * nv, 0.01);

    vec4 shallowColor = vec4(0.0, 0.64, 0.68, 1.0);
    shallowColor = vec4(0.27, 0.69, 0.68, 1.0);
    shallowColor = vec4(0.1, 0.39, 0.48, 1.0);
    vec4 deepColor = vec4(0.02, 0.05, 0.10, 1.0);
	vec4 ambient = mix(shallowColor, deepColor, CalculateHeight(intersectionPos.xz, variation).y);

	losMod = texture2D(losMap, losCoords.st).a;
	losMod = losMod < 0.03 ? 0.0 : losMod;

	vec4 color = vec4(0.0, 0.0, 0.0 , 1.0);
    //color += ambient;

	//float diffuse = max(0, dot(l, n));
	////color.rgb += diffuse;

	float specular2 = max(0, pow(dot(r,v), 8));
	//color.rgb += specular;

    vec4 reflection = ComputeReflection(n*vec3(0.1,1,0.1)); // Using the normal 0 1 0 works
    vec4 refraction = vec4(LightAttenuation(ComputeRefraction(n, v), waterDepth), 1.0);
    //refraction = vec4(ComputeRefraction(n, v), 1.0) * shallowColor;

    vec3 reflected = reflect(v, n);
    vec3 reflectedV = vec3(invV * vec4(reflected, 0));
    vec3 skyReflection = textureCube(skyCube, reflectedV).rgb;
    float factor = max(0, exp(0.1*waterDepth));// mix using the relative depth
	//vec4 refractionColor = refraction + shallowColor * exp(0.01*deepColor);
	vec4 refractionColor = mix(refraction, shallowColor, factor);
    refractionColor = refraction;
	vec4 reflectionColor = vec4(mix(skyReflection, reflection.rgb, reflection.a), 1.0);
	//vec4 amb = mix(refractionColor, reflectionColor, 3*F(F0, v, n));
	//vec4 amb = mix(refractionColor, reflectionColor, F(F0, v, n)) + 0.1 *
    //    ambient;
	vec4 amb = mix(refractionColor, reflectionColor, F(F0, n*vec3(0.2,1,0.2), n)) + 0.1 *
        ambient;
 
    gl_FragColor = vec4(skyReflection.rgb, 1.0);
    //gl_FragColor = vec4(n.rgb, 1.0);

    gl_FragColor = amb + specular;
    //gl_FragColor = refractionColor;
    //gl_FragColor = vec4(specular, specular, specular, 1.0);
	gl_FragColor = vec4(gl_FragColor.rgb * losMod, 1.0);
}

// Compute the normal given the world position coordinates xz
vec3 CalculateNormal(vec2 uv) {
    //vec3 n = texture2D(normalMap1, scale.x * uv).rgb;
	//return 2.0*n - 1.0;

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

	// --------------------------------------------------------------------------------------
	/*

    float ts = time;
    ////t = 1;

    vec3 bn = texture2D(normalMap1, scale.x * uv +
            wind1 * timeScale1 * ts).rgb * amplitude1;
    bn += texture2D(normalMap2, scale.x * uv +
            wind2 * timeScale2 * ts).rgb * amplitude2;
    bn += texture2D(normalMap3, scale.x * uv +
            wind3 * timeScale3 * ts).rgb * amplitude3;
    //bn.yz = bn.zy;

    ////return normalAttenuation * (2.0 * normalize(n * variation) - 1.0);
    ////return (2.0 * normalize(n * variation) - 1.0);
	//vec3 normal = n;
	//normal.yz = n.zy;
    ////return normalize(2.0 * normal - 1.0);
    //return normalize(2.0 * normal - 3.0);
    ////return normalize(normal);
    
    // Gram-Schmidt process to re-orthogonalize the vectors
    vec3 n = normalize(normal);
    vec3 t = normalize(tangent);
    t = normalize(t - dot(t, n) * n);

    // Fetch the normal from the normal map
    vec3 bitangent = cross(t, n);
    //vec3 bumpMapNormal = texture(normalMap, intersectionPos.xz + t_cst *
    //        vec2(t)).xyz;
    vec3 bumpMapNormal = 2.0 * bn - 3.0;

    // Create TBN matrix
    mat3 tbn = mat3(t, bitangent, n);

    // Transform the bumped normal int local space, dampen it and transform it
    // into the eye space.
    vec3 newNormal = tbn * bumpMapNormal;
    newNormal = normalize(newNormal);//normalize(a * newNormal);
    newNormal.yz = newNormal.zy;
    return vec3(V * vec4(newNormal, 0));

	*/
}



vec3 CalculateHeight(vec2 uv, float variation) {
    float t = time;
    //t = 1;

    vec3 h = texture2D(heightMap1, scale.x * uv + wind1 *
            timeScale1 * t).rgb * amplitude1 - 0.5;

    h += texture2D(heightMap2, scale.x * uv + wind2 *
            timeScale2 * t).rgb * amplitude2 - 0.5;

    h += texture2D(heightMap3, scale.x * uv + wind3 *
            timeScale3 * t).rgb * amplitude3 - 0.5;

    //return normalize(h);

    //vec3 h = texture2D(heightMap1, scale.x * uv).rgb - 0.5;
    //h.y *= 2;
    //h.xz *= 3;
    return h * variation;
    //return h;
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

float Sum(vec4 t)
{
	return t.x + t.y + t.z + t.w;
}

// Compute the reflection color
// The intersection between the far-clip of the reflection camera
// is used to find the texture coordinates
vec4 ComputeReflection(vec3 n)
{
    vec3 wavePos = intersectionPos;
    vec3 v = normalize(wavePos - cameraPos);
    vec3 r = reflect(v, normalize(n)); // n should be normalized

	// Use the Pluecker matrix to compute the intersection
	// see https://math.stackexchange.com/questions/2433207#2434182
	// and https://en.wikipedia.org/wiki/Pl%C3%BCcker_matrix#Intersection_with_a_plane
	vec4 p = vec4(wavePos, 1);
	vec4 q = vec4(r, 0);
	vec4 s = vec4(reflectionFarClipN, reflectionFarClipD);

	vec4 i = p*Sum(q*s) - q*Sum(p*s);
	vec3 iW = i.xyz/i.w;

	vec4 farP = reflectionMVP * vec4(iW, 1);

    float reflShiftUp = 1 / screenHeight;
	vec2 uv = vec2(0.5 * (farP.x / farP.w + 1), 0.5 * (farP.y / farP.w + 1) + reflShiftUp);
	
    vec4 reflection = texture2D(reflectionMap, uv);
    return reflection;
}

// Compute the refraction color, based on the paper from 
// Wymann "An approximate Image-Space Approach for Interactive Refraction"
vec3 ComputeRefraction(vec3 n, vec3 v)
{
    vec3 P1 = intersectionPos;
    vec3 K1 = GetWorldPosFromDepth();
    vec3 K2 = GetWorldFromHeightMap(P1.xz);

    vec3 r = refract(v, n, RATIO);
    vec3 T1 = r;

    float theta_t = acos(dot(-n, r));
    float theta_i = acos(dot(-v, n));
    float theta_ratio = theta_t / theta_i;

    float d_V = length(K1 - P1);
    float d_N = length(K2 - P1);
    float d_tilde = mix(d_V, d_N, theta_ratio);
    vec3 P2 = P1 + d_V * normalize(K1-K2);
    P2 = P1 + d_N * 0.5 * d_V;
    P2 = P1 + d_tilde * normalize(T1);

    vec4 I = refractionMVP * vec4(P2, 1.0);

    float refractionShiftUp = 1/screenHeight;
    vec2 uv = vec2(0.5 * (I.x / I.w + 1), 0.5 * (I.y / I.w + 1) +
            refractionShiftUp);
    vec3 refraction = texture2D(refractionMap, uv).rgb;

	if((refraction.r > 0.9 && refraction.g < 0.1 && refraction.b < 0.1) ||
       (refraction.r < 0.1 && refraction.g < 0.1 && refraction.b < 0.1))
    {
        I = refractionMVP * vec4(P1, 1.0);
        uv = vec2(0.5 * (I.x / I.w + 1), 0.5 * (I.y / I.w + 1) +
                refractionShiftUp);
        refraction = texture2D(refractionMap, uv).rgb;
    }

    waterDepth = length(P2 - P1);
    return refraction;
}

// Exponential light attenuation function
vec3 LightAttenuation(vec3 color, float depth)
{
    float red = exp(-depth*0.55);
    float green = exp(-depth*0.051);
    float blue = exp(-depth*0.02);

    return color * vec3(red, green, blue);
}

// From https://stackoverflow.com/questions/22360810/
// Reconstruct the world position from the depth buffer
vec3 GetWorldPosFromDepth()
{
    vec4 coordsCS;
    //vec2 texC = gl_FragCoord.xy/viewport;
    vec2 uv = gl_FragCoord.xy/viewport;
    coordsCS.xy = uv * 2.0 - 1.0;
    coordsCS.z = texture2D(entireSceneDepth, uv).r * 2.0 - 1.0;
    coordsCS.w = 1.0;

    vec4 coordsHS = invFullMVP * coordsCS;
    return coordsHS.xyz / coordsHS.w;

}

// Compute the terrain world elevation at a world point (x, z) based on the
// terrain height map
vec3 GetWorldFromHeightMap(vec2 worldXZ)
{
	vec2 uv = worldXZ/terrainWorldSize;
	float h = texture2D(terrain, uv).r;
	// Transform from the [0,1] range to the world range
    float worldHeight = h * (maxTerrainElevation - minTerrainElevation) +
        minTerrainElevation;
	return vec3(worldXZ.x, worldHeight, worldXZ.y);
}

// From realtimerendering.com/intersections
vec3 LineLineIntersection(vec3 o1, vec3 d1, vec3 o2, vec3 d2)
{
    vec3 d1xd2 = cross(d1,d2);
    float lengthD1xD2 = length(d1xd2);
    float denom = lengthD1xD2 * lengthD1xD2;

    //if(denom <= 0) return vec3(0,0,0)

    float t1 = determinant(mat3(o2-o1, d2, d1xd2)) / denom;

    return o1 + t1*d1;
}

// Because 0 A.D. uses glsl 1.2 ...
// From https://github.com/g-truc/glm/blob/master/glm/detail/func_matrix.inl
float determinant(mat3 m)
{
    return
        + m[0][0] * (m[1][1] * m[2][2] - m[2][1] * m[1][2])
        - m[1][0] * (m[0][1] * m[2][2] - m[2][1] * m[0][2])
        + m[2][0] * (m[0][1] * m[1][2] - m[1][1] * m[0][2]);
}
