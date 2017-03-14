$input a_position, a_color, a_texcoord0, i_data0, i_data1
$output v_wpos, v_texcoord0, v_texcoord1, v_common

#include "common.sh"

uniform mat4 u_cloudMatrix;

void main()
{
	vec4 up2 = vec4(u_invView[0][1], u_invView[1][1], u_invView[2][1], u_invView[3][1]);
	vec4 right2 = -vec4(u_invView[0][0], u_invView[1][0], u_invView[2][0], u_invView[3][0]);

	float rot = 0; //i_data1.y;
	float c = cos(rot);
	float s = sin(rot);
	
	vec4 up = c * up2 + s * right2;
	vec4 right = -s * up2 + c * right2;
	

	vec3 wpos = mul(u_cloudMatrix, vec4(i_data0.xyz, 1)).xyz + (up * a_position.y + right * a_position.x).xyz * i_data0.w;

	v_wpos = wpos;
	v_texcoord0 = a_texcoord0;
	
	v_common = i_data1.xyz;
	gl_Position = mul(u_viewProj, vec4(wpos, 1.0) );	
}
