#version 330 core

layout (location = 0) in vec2  aUnitVert;
layout (location = 1) in uvec2 aPos;
layout (location = 2) in uint  aId;
layout (location = 3) in uvec3 aFgCol;
layout (location = 4) in uvec3 aBgCol;

out vec2 TexCoords;

uniform uvec2 ScreenResPix;
uniform uvec2 CellSizePix;

uniform samplerBuffer AtlasUVsLookup;
uniform isamplerBuffer CharFormatLookup;

void main() 
{
	ivec4 format = texelFetch(CharFormatLookup, int(aId));
	ivec2 char_size = format.xy;
	ivec2 char_bearing = format.zw;

	vec2 pix_pos = char_size * aUnitVert + aPos + vec2(char_bearing.x, int(CellSizePix.y) - char_bearing.y);
	vec2 norm_pos = vec2(1., -1.) * (2. * pix_pos - ScreenResPix) / ScreenResPix;
	
	vec4 atlas_uv_bords = texelFetch(AtlasUVsLookup, int(aId));
	TexCoords = mix(atlas_uv_bords.xy, atlas_uv_bords.zw, aUnitVert);

	gl_Position = vec4(norm_pos, 0.0, 1.0);
}
