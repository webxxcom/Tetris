#version 330 core
out vec4 FragColor;

in vec2 xTileCoord;

const float tileSide = 0.125f;

uniform sampler2D texture1;
uniform uint tileColor;
uniform float transparency;

void main()
{
	vec4 textr = texture(texture1, vec2(xTileCoord.x + tileSide * tileColor, xTileCoord.y)); 
	FragColor = vec4(textr.xyz, textr.w * (1 - transparency));
}