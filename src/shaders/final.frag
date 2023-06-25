// Fragment shader
//#version 150
#version 330


uniform sampler2D u_colorTex;
uniform sampler2D u_aoTex;



uniform mat4 u_matP;

uniform int u_occlusion_type; // 1 = SSAO, 2 = SSDO
	
// INPUT	

in vec3 vert_uv;



// OUTPUT
out vec4 frag_color;




// MAIN
void main()
{
	
	vec3 color = texture(u_colorTex, vert_uv.xy).rgb;
	vec4 ao = texture(u_aoTex, vert_uv.xy).rgba;

	if(u_occlusion_type == 1)
		frag_color = vec4(color.rgb * ao.rgb, 1.0);
	if(u_occlusion_type == 2)
		frag_color = vec4(color.rgb + ao.rgb, 1.0);



}

