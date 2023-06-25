// Fragment shader
//#version 330 core
#version 410


//uniform sampler2D u_albedoTex;
//uniform vec3 u_diffuseColor;
//uniform int u_useAlbedoTex;

// Ouput data
layout (location = 0) out vec3 gPosition;
layout (location = 1) out vec3 gNormal;
layout (location = 2) out vec3 gColor;


in vec3 vert_uv;
in vec3 pos_view;
in vec3 vecN_view;

void main()
{
	vec3 albedo;
	/*if(u_useAlbedoTex == 1)
		albedo = texture(u_albedoTex, vert_uv.xy).rgb;
	else
		albedo = u_diffuseColor;*/
	
	gColor = albedo;
	
	// store the fragment position vector in the first gbuffer texture
    //gPosition = vec3(pos_view.x, pos_view.y, -pos_view.z);
	gPosition = pos_view;
    // also store the per-fragment normals into the gbuffer
    //gNormal = normalize(vec3(vecN_view.x, vecN_view.y, -vecN_view.z));	
	gNormal = normalize(vecN_view);
}