#version 330 core

layout (location = 0) in vec2  aUnitVert;
layout (location = 1) in uvec2 aPos;
layout (location = 2) in int   aId;
layout (location = 3) in uvec4 aFgCol;
layout (location = 4) in uvec4 aBgCol;

out vec2 TexCoords;

uniform uvec2 ScreenResPix;
uniform uvec2 CellSizePix;

uniform samplerBuffer AtlasUVsLookup;

void main() 
{
	vec2 pix_pos = CellSizePix * aUnitVert + aPos;
	vec2 norm_pos = vec2(1., -1.) * (2. * pix_pos - ScreenResPix) / ScreenResPix;
	
	vec4 atlas_uv_bords = texelFetch(AtlasUVsLookup, aId);
	TexCoords = mix(atlas_uv_bords.xy, atlas_uv_bords.zw, aUnitVert);

	gl_Position = vec4(norm_pos, 0.0, 1.0);
}
