// Fragment shader
#version 330


// ------------------------------------------------------------------------------------------------
// - Render a geometry defined by G-buffers (Position and normal textures)
// - Compute Screen-Space Light Reflection (SSLR)
// This shader is used to generate SSLR map.
//
// Based on Screen-Space Directionnal Occlusion by T.Ritschel et al., and implementation by J. Huber:
//
//		Ritschel et al., "Approximating dynamic global illumination in image space", I3D 2009
// 		https://doi.org/10.1145/1507149.1507161
//
// 		J. Huber, "Enhancing Visual Rendering for Interactive AR". Msc. thesis, 2019.
// ------------------------------------------------------------------------------------------------


// UNIFORMS
uniform sampler2D u_screenTex;
uniform sampler2D u_noiseTex;
uniform sampler2D u_posTex;
uniform sampler2D u_normalTex;
uniform samplerCube u_cubemap;
uniform vec3 u_samples[64];
uniform mat4 u_matP;
uniform mat4 u_matV;
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


const float PI = 3.14159265359;

// MAIN
void main()
{
	vec2 noiseScale = vec2(u_screenWidth/4.0, u_screenHeight/4.0);
	radius = u_radius;
	radius = 1.0;
	
	// read fragment 3D pos from G-buffer position texture
	vec3 fragPos = texture(u_posTex, vert_uv.xy).xyz;
	// read fragment 3D normal from G-buffer normal texture
	vec3 normal = texture(u_normalTex, vert_uv.xy).rgb;
	
	
	vec3 directionalLight = vec3(0.0);
	vec3 indirectLight = vec3(0.0);	
	
	// ignore fragment if normal or position is empty
	if (normal != vec3(0.0f) && fragPos != vec3(0.0f) ) 
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
			

			float sampleDepth = texture(u_posTex, offset.xy).z; // get depth value at kernel sample
			vec3 sampleNormal = texture(u_normalTex, offset.xy).rgb;
			vec3 samplePos2 = texture(u_posTex, offset.xy).xyz;
			vec3 sampleColor = texture(u_screenTex, offset.xy).xyz; 
			
			if (sampleDepth < samplePos.z || radius < abs(fragPos.z - sampleDepth))
			{
				// Directionnal Light : Irradiance from cubemap (if any)
				vec4 skyboxDirection = inverse(u_matV) * vec4(samplePos - fragPos, 0.0);
				vec3 skyboxColor = texture(u_cubemap, skyboxDirection.xyz).xyz;
				directionalLight += skyboxColor * dot(normal, normalize(samplePos - fragPos));
				
			}
			else
			{
				// Indirect light: approximate one step of screen-space reflection
				vec3 receiverToSender = fragPos.xyz - samplePos2.xyz;
				vec3 normalizedRS = normalize(receiverToSender);
				indirectLight += max(0.0, dot(normalizedRS, -normal)) * max(0.0, dot(normalizedRS, sampleNormal)) * (1 / max(radius*radius*0.5, dot(receiverToSender, receiverToSender))) * 0.5 * sampleColor;
			}
			
		}  


	}


	directionalLight = PI * directionalLight / kernelSize;
	frag_color = vec4( directionalLight.rgb, 1.0);

	frag_color.rgb += indirectLight;


}

