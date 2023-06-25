// Fragment shader
#version 330


// UNIFORMS

uniform sampler2D u_albedoTex;

// INPUT	
in vec3 vert_uv;


// OUTPUT
out vec4 frag_color;




void main()
{
	
	vec4 color = texture(u_albedoTex, vert_uv.xy);
	
	
	frag_color = color;

}