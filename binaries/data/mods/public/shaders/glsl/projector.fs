#version 120

// Schlick's R(theta=0) = ((1-1.333)/(1+1.333))^2
#define F0 0.02037318784
#define PI 3.14159265358979

// air/water = 1/1.333
#define RATIO 0.7501875469

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
uniform sampler2D reflectionMapDepth;
uniform vec3 reflectionFarClipN;
uniform float reflectionFarClipD;
uniform mat4 reflectionMVP; 

uniform sampler2D refractionMap;
uniform vec3 refractionFarClipN;
uniform float refractionFarClipD;
uniform mat4 refractionMVP; 

uniform mat4 invV;
uniform mat4 invTransform;

uniform float screenWidth;
uniform float screenHeight;
uniform float nearPlane;
uniform float farPlane;

uniform vec3 ambient;
uniform vec3 sunDir;
uniform vec3 sunColor;
uniform vec3 cameraPos;
uniform float time;

uniform samplerCube skyCube;

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

//const vec3 normalAttenuation = vec3(0.3, 1, 0.3);

vec3 CalculateNormal(vec2 uv, float variation);
vec3 CalculateHeight(vec2 uv, float variation);
float F(float F_0, vec3 v, vec3 h);
float G_smith(float nx, float k);
float G(float nl, float nv, float k);
float D(float nh, float alpha2);
//float DistanceToPlane(vec3 p, vec3 normal, float D);
//vec3 FindLineSegIntersection(vec3 start, vec3 end, vec3 planeNormal,
//        float planeD);
//vec3 FindRayIntersection(vec3 start, vec3 direction, vec3 planeNormal,
//        float planeD);
//vec2 GetScreenCoordinates(vec3 world);
vec4 ComputeReflection(vec3 n);
vec3 ComputeRefraction(vec3 n);
float Sum(vec4 t);
vec3 LightAttenuation(vec3 color, float depth);

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
    vec3 normalAttenuation = vec3(0.1, 1, 0.1);
	vec3 n = normalize(normal);
    n = CalculateNormal(waterCoords.xz, variation);
    //n = vec3(0, 1, 0);
    //n = texture2D(heightMap1, 0.01*waterCoords.xz).rgb;

    vec3 l = normalize(sunDir);
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
    vec4 deepColor = vec4(0.02, 0.05, 0.10, 1.0);
	vec4 ambient = mix(shallowColor, deepColor, CalculateHeight(intersectionPos.xz, variation).y);

	losMod = texture2D(losMap, losCoords.st).a;
	losMod = losMod < 0.03 ? 0.0 : losMod;

	vec4 color = vec4(0.0, 0.0, 0.0 , 1.0);
    //color += ambient;

	//float diffuse = max(0, dot(l, n));
	////color.rgb += diffuse;

	float specular2 = max(0, pow(dot(r,v), 18));
	//color.rgb += specular;

    vec4 reflection = ComputeReflection(normal);
    vec4 refraction = vec4(ComputeRefraction(n), 1.0);

    vec3 reflected = reflect(v, n);
    vec3 reflectedV = vec3(invV * vec4(reflected, 0));
    vec3 skyReflection = textureCube(skyCube, reflectedV).rgb;
	vec4 refractionColor = refraction;
	vec4 reflectionColor = vec4(mix(skyReflection, reflection.rgb, reflection.a), 1.0);
	vec4 amb = mix(refractionColor, reflectionColor, F(F0, v, n));
	//---------------------------------------------------------------------------------------------------------

    vec3 test = texture2D(reflectionMapDepth, 0.01*intersectionPos.xz).rgb;
	//color += amb;
	/////*
	//color = amb;
    ////color = vec4(reflected, 1.0);
    ////color = amb;
    float c = F(F0, v, vec3(0,1,0));
    c = specular;
	//float c = 1.0;
    //c = F_val;
	color = vec4(c, c, c, 1.0);
    ////color = ambient + specular;
	//color = vec4(n, 1.0);
	//color = reflection;
	color = amb + specular2;
	//*/
	//color = vec4(CalculateHeight(intersectionPos.xz, variation), 1.0);
	//color = vec4(n, 1.0);
    //color = vec4(test, 1.0);
	gl_FragColor = color * losMod;
}

// Convert the normal form the normal map to the eye space. Attenuate it by a
// factor a and attenuate the time scroll with a factor t_cst
/*
vec3 CalculateNormal(vec3 a, float t_cst, sampler2D normalMap) {

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

vec3 CalculateNormal(vec2 uv, float variation) {
    //vec3 n = texture2D(normalMap1, scale.x * uv).rgb;

    vec3 n = texture2D(normalMap1, scale.x * uv +
            wind1 * timeScale1 * time).rgb * amplitude1;
    n += texture2D(normalMap2, scale.x * uv +
            wind2 * timeScale2 * time).rgb * amplitude2;
    n += texture2D(normalMap3, scale.x * uv +
            wind3 * timeScale3 * time).rgb * amplitude3;

    //return normalAttenuation * (2.0 * normalize(n * variation) - 1.0);
    //return (2.0 * normalize(n * variation) - 1.0);
	vec3 normal = n;
	normal.yz = n.zy;
    //return normalize(2.0 * normal - 1.0);
    return normalize(2.0 * normal - 3.0);
    //return normalize(normal);
}

vec3 CalculateHeight(vec2 uv, float variation) {
    //vec3 h = texture2D(heightMap1, scale.x * uv + wind1 *
    //        timeScale1 * time).rgb * amplitude1 - 0.5;

    //h += texture2D(heightMap2, scale.x * uv + wind2 *
    //        timeScale2 * time).rgb * amplitude2 - 0.5;

    //h += texture2D(heightMap3, scale.x * uv + wind3 *
    //        timeScale3 * time).rgb * amplitude3 - 0.5;

    //return normalize(h);

    vec3 h = texture2D(heightMap1, scale.x * uv).rgb - 0.5;
    h.y *= 2;
    h.xz *= 3;
    return h;
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

//vec3 FindLineSegIntersection(vec3 start, vec3 end, vec3 planeNormal,
//        float planeD)
//{
//	float dist1 = DistanceToPlane(start, planeNormal, planeD);
//	float dist2 = DistanceToPlane(end, planeNormal, planeD);
//
//	float t = (-dist1) / (dist2-dist1);
//
//	return mix(start, end, t);
//}

//float DistanceToPlane(vec3 p, vec3 normal, float D)
//{
//	//normal * p + waterD;
//	return normal.x * p.x + normal.y * p.y + normal.z * p.z + D;
//}
//
//vec3 FindRayIntersection(vec3 start, vec3 direction, vec3 planeNormal,
//        float planeD)
//{
//    float dot = dot(planeNormal, direction);
//
//    if(dot == 0.0f) return vec3(0, 0, 0); // Correct??? What should be returned?
//    float t = -(planeNormal.x * start.x + planeNormal.y * start.y +
//            planeNormal.z * start.z + planeD);
//    t /= dot;
//    vec3 intersection = start + t * direction;
//    //vec3 intersection = start - (direction * ( DistanceToPlane(start,
//    //                planeNormal, planeD) / dot));
//    return intersection;
//}

//vec2 GetScreenCoordinates(vec3 world)
//{
//    vec4 screenSpace = reflectionMVP * vec4(world, 0.0);
//
//    vec2 coordinates = screenSpace.xz / screenSpace.w;
//    coordinates.x = 0.0;//(coordinates.x + 1) * 0.5 ;//screenHeight/screenWidth;
//    //xy.y = (1 - xy.y) * 0.5;//* screenHeight;
//    coordinates.y = ( (coordinates.y + 1) * 0.5 );// - 55.0/255.0;// - 53.0/255.0;//* screenHeight;
//
//
//    return coordinates;
//}

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

    //// We are inside the view frustrum of the reflection camera
    //if(dot(normalize(-reflectionFarClipN), normalize(reflected)) <=
    //        cos(0.5*reflectionFOV))
    //{
    //    vec3 farClipInter = FindRayIntersection(intersectionPos, reflected,
    //        reflectionFarClipN, reflectionFarClipD);
    //    //vec3 coords = mat3(reflectionMVP) * farClipInter;
    //    vec4 coords = reflectionMVP * vec4(farClipInter, 1.0);
    //    coords.xyz /= coords.w;
    //    vec2 texCoords = coords.xy;//(reflectionMVP * vec4(farClipInter, 1.0)).xy;
    //    texCoords = (texCoords + 1.0) * 0.5;
    //    texCoords.y += 1/screenHeight;
    //    reflection += texture2D(reflectionMap, texCoords);
    //}
}

// Compute the refraction color
// The intersection between the far-clip of the refraction camera
// is used to find the texture coordinates
vec3 ComputeRefraction(vec3 n)
{
    vec3 wavePos = intersectionPos;
    vec3 v = normalize(wavePos - cameraPos);
    vec3 r = refract(v, normalize(n), RATIO); // n should be normalized

	vec4 p = vec4(wavePos, 1);
	vec4 q = vec4(r, 0);
	vec4 s = vec4(refractionFarClipN, refractionFarClipD);

	vec4 i = p*Sum(q*s) - q*Sum(p*s);
	vec3 iW = i.xyz/i.w;

	vec4 farP = refractionMVP * vec4(iW, 1);
	//vec4 farP = refractionMVP * vec4(wavePos + 15*r, 1);

    float refrShiftUp = 1 / screenHeight;
	vec2 uv = vec2(0.5 * (farP.x / farP.w + 1), 0.5 * (farP.y / farP.w + 1) + refrShiftUp);
	
    vec3 refraction = texture2D(refractionMap, uv).rgb;

    return refraction;
}

vec3 LightAttenuation(vec3 color, float depth)
{
    float red = exp(-depth*0.3);
    float green = exp(-depth*0.051);
    float blue = exp(-depth*0.046);

    return color * vec3(red, green, blue);
}


