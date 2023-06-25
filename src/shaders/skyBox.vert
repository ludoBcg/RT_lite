// Vertex shader
#version 330
//#extension GL_ARB_explicit_attrib_location : require


// VERTEX ATTRIBUTES
layout(location = 0) in vec4 a_position;

// UNIFORMS
uniform mat4 u_view;		// view matrix
uniform mat4 u_projection;	// projection matrix

// OUTPUT
out vec3 vert_uv;

void main()
{

	vert_uv = vec3(a_position.xyz);
	
	vec4 pos = u_projection * u_view * a_position;
	gl_Position = pos.xyww;

}
