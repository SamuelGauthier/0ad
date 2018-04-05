#version 120

uniform sampler2D losMap;

varying vec2 losCoords;

void main()
{
	float losMod;


	losMod = texture2D(losMap, losCoords.st).a;
	losMod = losMod < 0.03 ? 0.0 : losMod;

	vec4 color = vec4(1, 0, 0, 1);
	gl_FragColor = color * losMod;
}
