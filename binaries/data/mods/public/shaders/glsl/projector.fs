#version 120

uniform sampler2D losMap;

uniform sampler2D height;

varying vec2 losCoords;
varying vec4 waterCoords;

void main()
{
	float losMod;


	losMod = texture2D(losMap, losCoords.st).a;
	losMod = losMod < 0.03 ? 0.0 : losMod;

	//vec4 color = vec4(1, 0, 0, 1);
    vec4 color = texture2D(height, 0.01 * waterCoords.xz);
	gl_FragColor = color * losMod;
}
