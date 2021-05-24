// Vertex shader
//#version 150
#version 330
//#extension GL_ARB_explicit_attrib_location : require

layout(location = 0) in vec4 a_position;
layout(location = 3) in vec2 a_uv;

uniform mat4 u_matM;
uniform mat4 u_matV;
uniform mat4 u_matP;


out vec3 vert_uv;


void main()
{

	//gl_Position = u_mvp * a_position;
	gl_Position = a_position;
	
	vert_uv = vec3(a_uv.x, a_uv.y, 0.0);
}
