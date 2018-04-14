#version 120

uniform sampler2D losMap;

uniform sampler2D height;

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
    //vec4 color = mix(shallowColor, deepColor, waterHeight);
    //vec4 test = normalize(waterCoords);
    //vec4 color = vec4(waterHeight, 0, 0, 1);
	gl_FragColor = color * losMod;
}
