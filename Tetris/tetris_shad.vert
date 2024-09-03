#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aTextrCoord;
layout (location = 2) in vec2 aTileCoord;

out vec2 xTextrCoord;
out vec2 xTileCoord;

uniform mat4 model;

void main()
{
	gl_Position = model * vec4(aPos, 1.0);
	xTextrCoord = aTextrCoord;
	xTileCoord = aTileCoord;
}