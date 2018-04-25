#version 120

uniform sampler2D losMap;

uniform sampler2D height;
uniform sampler2D heightMap1;

varying vec2 losCoords;
varying vec4 waterCoords;
varying float waterHeight;

void main()
{
	float losMod;

    vec4 shallowColor = vec4(0.0, 0.64, 0.68, 0.5);
    vec4 deepColor = vec4(0.02, 0.05, 0.10, 1.0);

	losMod = texture2D(losMap, losCoords.st).a;
	losMod = losMod < 0.03 ? 0.0 : losMod;

	vec4 color = vec4(1, 1, 1, 1);
    //vec4 color = texture2D(height, 0.01 * waterCoords.xz);
    //vec4 color = deepColor;
    //float alpha = mix(1.0, 0.0, waterHeight + 0.5);
    //color = mix(shallowColor, deepColor, waterHeight + 0.5);
    //color.a = alpha;
    //color = texture2D(heightMap1, 0.01 * waterCoords.xz);
    float t = texture2D(heightMap1, 0.01 * waterCoords.xz).g;
    //float t = waterHeight;
    //color = vec4(t, t, t, 1);
    color = vec4(1, 0, 0, 1);
    //float dh = waterHeight + 0.5;
    //color = vec4(dh, dh, dh, 1);
    //vec4 test = normalize(waterCoords);
    //vec4 color = vec4(waterHeight, 0, 0, 1);
	gl_FragColor = color * losMod;
}
