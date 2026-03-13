#version 330 core

in vec2 TexCoords;

out vec4 FragColor;

uniform sampler2D AtlasTexture;

void main()
{
	float alpha = texture(AtlasTexture, TexCoords).r;
	FragColor = alpha > 0. ? vec4(1., 1., 1., 1.0) : vec4(0., 0., 0., 1.); 
}
