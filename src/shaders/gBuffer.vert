// Vertex shader
//#version 330 core
#version 330

layout(location = 0) in vec4 a_position;
layout(location = 1) in vec3 a_normal;
layout(location = 3) in vec2 a_uv;


uniform mat4 u_matM;
uniform mat4 u_matV;
uniform mat4 u_matP;


out vec3 vert_uv;
out vec3 vecN_view;
out vec3 pos_view;

void main()
{

	// compute Model-View and Model-View-Projection matrices
	mat4 matMV = u_matV * u_matM;
	mat4 matMVP = u_matP * matMV;
	
	// Normal in view coords
	mat3 normalMatrix = transpose(inverse(mat3(u_matM)));
	//vec4 normal = matMV * vec4(a_normal.xyz, 1.0);
	//vecN_view = vec3(normal.xyz);
	vecN_view = normalMatrix * a_normal;
	
	// Normal in view coords
	vec4 pos = /*matMV*/u_matM * a_position;
	pos_view = vec3(pos.xyz);
	
	// vertex UV
	vert_uv = vec3(a_uv.x, 1.0 - a_uv.y, 0.0);
	
	// project vertices to view space
	gl_Position = matMVP * a_position;	
	
}
