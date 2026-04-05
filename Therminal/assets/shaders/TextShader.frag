#version 330 core

in vec2 TexCoords;

out vec4 FragColor;

uniform sampler2D AtlasTexture;

void main()
{
	float alpha = texture(AtlasTexture, TexCoords).r;
	FragColor = vec4(alpha, alpha, alpha, alpha);
}
