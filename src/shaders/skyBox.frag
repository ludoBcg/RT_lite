// Fragment shader
#version 330


// UNIFORMS
uniform samplerCube u_cubemap;
	
// INPUT	
in vec3 vert_uv;

// OUTPUT
out vec4 frag_color;




void main()
{
	
	frag_color = texture(u_cubemap, vert_uv);
}