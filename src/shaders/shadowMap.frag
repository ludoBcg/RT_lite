// Fragment shader
//#version 330 core
#version 330

// Ouput data
layout(location = 0) out float frag_depth;



void main()
{
	// only output is the depth of the fragment
	frag_depth = gl_FragCoord.z;
	
}