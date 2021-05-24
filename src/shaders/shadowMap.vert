// Vertex shader
//#version 330 core
#version 330

layout(location = 0) in vec4 a_position;


uniform mat4 u_lvp;

void main()
{

	// project vertices to light space
	gl_Position = u_lvp * a_position;	
}
