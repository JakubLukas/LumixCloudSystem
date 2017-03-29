$input v_wpos, v_texcoord0, v_texcoord1, v_common // in...

#include "common.sh"

SAMPLER2D(u_texColor, 0);
SAMPLER2D(u_texShadowmap, 14);
SAMPLER2D(u_depthBuffer, 15);
uniform mat4 u_shadowmapMatrices[4];
uniform vec4 u_ambientColor;
uniform vec4 u_lightRgbAttenuation;

void main()
{
	//const float SOFT_MULTIPLIER = 5.0;
	//vec3 screen_coord = getScreenCoord(v_wpos);
	
	//float depth = texture2D(u_depthBuffer, screen_coord.xy * 0.5 + 0.5).x;

	//float depth_diff = toLinearDepth(screen_coord.z) - toLinearDepth(depth);
	
	vec4 col = vec4(1,1,1,0.1) /*texture2D(u_texColor, v_texcoord0)*/ * vec4(v_common, 1);
	
	//vec3 diffuse = col.rgb * u_lightRgbAttenuation.rgb * directionalLightShadow(u_texShadowmap, u_shadowmapMatrices, vec4(v_wpos, 1.0), 1); 
	//vec3 ambient = col.rgb * u_ambientColor.rgb;
			
	//col.rgb = diffuse + ambient;
	//col.a *= clamp(depth_diff * SOFT_MULTIPLIER, 0, 1);
	gl_FragColor.rgba = col;
}
