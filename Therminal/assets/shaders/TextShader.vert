#version 330 core

layout (location = 0) in vec2  aVert;
layout (location = 1) in int   aId;
layout (location = 2) in uvec2 aPos;
layout (location = 3) in uvec4 aFg;
layout (location = 4) in uvec4 aBg;

//out int  id;
//out vec2 vert;
//out vec4 fg;
//out vec4 bg;

uniform uvec2 ScreenRes;

void main() 
{
	//vec2 ndc = vec2(apos) / vec2(800., 600.) - vec2(1., 1.);
	gl_Position = vec4(aVert, 0.0, 1.0);
	//id = aid;
	//vert = avert;
	//fg = afg;
	//bg = abg;
}
