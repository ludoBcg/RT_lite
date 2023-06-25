// Fragment shader
#version 330


// ------------------------------------------------------------------------------------------------
// - Render a geomtery defined by G-buffers (Position and normal textures)
// - Compute Screen-Space Ambient Occlusion (SSAO)
// This shader is used to generate SSAO map.
//
// Based on the following SSAO tutorial:
// 		https://learnopengl.com/Advanced-Lighting/SSAO
// ------------------------------------------------------------------------------------------------


// UNIFORMS
uniform sampler2D u_noiseTex;
uniform sampler2D u_posTex;
uniform sampler2D u_normalTex;
uniform vec3 u_samples[64];
uniform mat4 u_matP;
uniform float u_radius;
uniform float u_screenWidth;
uniform float u_screenHeight;

	
// INPUT	
in vec3 vert_uv;


// OUTPUT
out vec4 frag_color;


// GLOBAL VARIABLES
vec3 coefficientSum = vec3(0);

// tile noise texture over screen, based on screen dimensions divided by noise size
//const vec2 noiseScale = vec2(1024.0/4.0, 720.0/4.0); // screen = 1024 x 720

//uniform AOparams params;
int kernelSize = 40;
float radius = 0.05;
float bias = 0.1;




// MAIN
void main()
{
	vec2 noiseScale = vec2(u_screenWidth/4.0, u_screenHeight/4.0);
	radius = u_radius;
	radius = 1.0;
	float occlusion = 0.0;
	
	// read fragment 3D pos from G-buffer position texture
	vec3 fragPos = texture(u_posTex, vert_uv.xy).xyz;
	// read fragment 3D normal from G-buffer normal texture
	vec3 normal = texture(u_normalTex, vert_uv.xy).rgb;
	
	// ignore fragment if normal or position is empty
	if (normal == vec3(0.0f) || fragPos == vec3(0.0f) ) 
	{
		occlusion = 1.0;
	}
	else
	{
		// build random direction vector from noise texture
		vec3 randomVec = texture(u_noiseTex, vert_uv.xy * noiseScale).xyz; 

		// build TBN matrix
		vec3 tangent = normalize(randomVec - normal * dot(randomVec, normal));
		vec3 bitangent = cross(normal, tangent);
		mat3 TBN = mat3(tangent, bitangent, normal); 


		for(int i = 0; i < kernelSize; ++i)
		{
			// get sample position
			vec3 samplePos = TBN * u_samples[i]; // from tangent to view-space
			samplePos = fragPos + samplePos * radius; 
			
			vec4 offset = vec4(samplePos, 1.0);
			offset = u_matP * offset;    	  		// from view to clip-space
			offset.xyz /= offset.w;               	// perspective divide
			offset.xyz  = offset.xyz * 0.5 + 0.5; 	// transform to range 0.0 - 1.0  
			
			float sampleDepth = texture(u_posTex, offset.xy).z;
			
			float rangeCheck = smoothstep(0.0, 1.0, radius / abs(fragPos.z - sampleDepth));
			occlusion += (sampleDepth >= samplePos.z + bias ? 1.0 : 0.0) * rangeCheck;  
			
		}  

		occlusion = 1.0 - (occlusion / kernelSize);
		occlusion = pow(occlusion, 0.5);

	}

	frag_color = vec4(occlusion);

}

