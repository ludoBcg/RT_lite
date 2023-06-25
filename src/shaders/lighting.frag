// Fragment shader
//#version 330


// ------------------------------------------------------------------------------------------------
// - Render a geometry using Cook-Torrance BRDF and Phisycally-Based Rendering (PBR)
// Based on LearnOpenGL tutorial:
//		https://learnopengl.com/PBR/Lighting
//
// - Compute Simulate Light Transmission to approximate Sub-Surface Scattering (SSS)
// Based on GPU Gems, and implementation by A. Brandstaetter:
//
//		S. Green, “Real-Time Approximations to Subsurface Scattering”. GPU Gems 1, chap 16.
//		https://developer.nvidia.com/gpugems/GPUGems/gpugems_ch16.html
//
//      A. Brandstaetter, "Real-time rendering of realistic surfaces using subsurface scattering". 
//		Bsc. thesis, 2020.
//
// - Compute shadow mapping, based on the following tutorials:
//		https://learnopengl.com/Advanced-Lighting/Shadows/Shadow-Mapping
//		http://www.opengl-tutorial.org/intermediate-tutorials/tutorial-16-shadow-mapping/
// ------------------------------------------------------------------------------------------------


// UNIFORMS
uniform vec3 u_lightColor;

uniform vec3 u_ambientColor;
uniform vec3 u_diffuseColor;
uniform vec3 u_specularColor;
uniform float u_specularPower;

uniform sampler2D u_albedoTex;
uniform sampler2D u_normalMap;
uniform sampler2D u_metalMap;
uniform sampler2D u_glossMap;
uniform sampler2D u_ambientMap;


uniform int u_useAmbient;
uniform int u_useDiffuse;
uniform int u_useSpecular;
uniform int u_useAlbedoTex;
uniform int u_useNormalMap;
uniform int u_usePBR;
uniform int u_useAmbMap;
uniform int u_useEnvMapReflec;
uniform int u_useEnvMapRefrac;
uniform int u_useShadowMap;
uniform int u_useGammaCorrec;
uniform vec3 u_lightPos;
uniform vec3 u_camPos;
uniform int u_isLightDir;
uniform float u_distLightMax;
uniform int u_useSimTransmit;
	
// INPUT

in vec3 vecN_world;
in vec3 vecV_world;
in vec3 vecT_world;
in vec3 vecBT_world;

in vec3 vert_uv;
in vec3 pos_world;

in vec4 pos_ls;


// OUTPUT
out vec4 frag_color;




// MAIN
void main()
{
	
	// 1- Get input vectors ------------------------------------------------
	
	// Final vectors used in lighting model
	vec3 l_vecN;
	vec3 l_vecL;
	vec3 l_vecV;
	
	
	// compute light vector, depending on light source type
	vec3 vecL_world;

	if(u_isLightDir == 1)
	{
		// cst vec if directional light
		vecL_world = normalize( mat3(u_matM) * u_lightPos);  // !!! L vec is from scene O to light source (should be from scene center)
	}
	else
	{
		// point light
		vecL_world = normalize( mat3(u_matM) * u_lightPos - pos_world);
	}


	if(u_useNormalMap == 1)
	{
		// if normal map, transfer all vectors to tangent space 
		
		// Read new normal from normal map
		l_vecN = texture(u_normalMap, vert_uv.xy).rgb * 2.0 - 1.0;
		l_vecN = normalize(l_vecN);		
		
		// compute TBN matrix
		mat3 TBN = transpose( mat3(vecT_world, vecBT_world, vecN_world) );
		// compute new version of L and V in tangent space
		l_vecL = normalize( TBN * vecL_world);
		l_vecV = normalize( TBN * vecV_world );
	}
	else
	{
		// if not, keep everything in world space
		l_vecN = vecN_world;
		l_vecL = vecL_world;
		l_vecV = vecV_world;
	}
	
	
	// 2- Compute shading -------------------------------------------------

	// init final color to white
	vec4 color = vec4(0.0f, 0.0f, 0.0f, 1.0f);
	

	// 2.1- Get albedo color -------------------------------------------
	
	// albedo map
	vec3 albedoD;
	vec3 albedoS;
	if(u_useAlbedoTex == 1)
	{
		albedoD = texture(u_albedoTex, vert_uv.xy).rgb;
		albedoS = albedoD;
	}
	else
	{
		albedoD = u_diffuseColor;
		albedoS = u_specularColor;
	}
	// environment map
	if(u_useEnvMapReflec == 1)
	{
		albedoS += ambient_reflection(l_vecN, l_vecV, u_specularPower, u_cubemap, 7);			
	}
	
	// ambient occlusion
	float ambOcc = 1.0;
	if(u_useAmbMap == 1)
	{
		ambOcc = texture(u_ambientMap, vert_uv.xy).r;
	}
	

	// 2.2- Get metalness and roughness coeffs  ------------------------
	
	float metalness;
	float roughness;
	float glossiness;
	if(u_usePBR == 1)
	{
		// Get metallic factor from metallicness texture
		metalness = texture(u_metalMap, vert_uv.xy).r;
		
		// Get gloss factor from glossiness texture
		roughness = texture(u_glossMap, vert_uv.xy).r;
		glossiness = 1.0f - roughness;
	}
	else
	{
		metalness = 0.5f;
		roughness = 1.0f - (u_specularPower / 2048.0f);
	}
	
	
	// 2.3- Compute Cook Torrance BRDF (f_r) ---------------------------
	
	// base reflectivity of the surface
	vec3 F0 = vec3(0.04);
	F0 = mix(F0, albedoS.rgb, metalness);
	
	// START for each light source (only one here)
	// [recompute l_vecL for currence light source]
	
	// Halfway vector
	vec3 l_vecH = normalize(l_vecL + l_vecV);
	

	// Cook Torrance BRDF
	vec3 f_r = vec3(0.0);
	vec3 f_diff = vec3(0.0);
	vec3 f_spec = vec3(0.0);
	
	// specular term
	
	float D = DistributionGGX(l_vecN, l_vecH, roughness);
	float G = GeometrySmith(l_vecN, l_vecV, l_vecL, roughness);
	vec3 F = fresnelSchlick(max(dot(l_vecH, l_vecV), 0.0), F0);

	vec3 numerator = D * F * G;
	float denominator = 4.0 * max(dot(l_vecN, l_vecV), 0.0) * max(dot(l_vecN, l_vecL), 0.0);

	vec3 f_CookTorrance = numerator / max(denominator, 0.001);


	// Lambertian diffuse term
	
	vec3 f_Lambert = albedoD / PI;
	

	vec3 k_s = F;
	vec3 k_d = vec3(1.0) - k_s/*f_CookTorrance*/;
	k_d *= 1.0 - metalness;
	

	f_diff = k_d * f_Lambert;
	f_spec = f_CookTorrance;
	/*if(u_useDiffuse == 1)
	{
		f_r += k_d * f_Lambert;
	}
	
	if(u_useSpecular == 1)
	{
		f_r += f_CookTorrance;
	}*/
	
	
	// 2.3- Compute reflectance equation -------------------------------
	
	vec3 Lo = vec3(0.0);
	
	// constant attenuation if directionnal light source
	float attenuation = 10.0f; 
	if(u_isLightDir == 0)
	{
		float distance = length( mat3(u_matM) * u_lightPos - pos_world ) / u_distLightMax;
		distance *= 0.5; // reduce distance to reduce attenuation
		attenuation = 1.0 / (distance * distance);
	}


	vec3 radiance = u_lightColor * attenuation;
	// add to outgoing radiance Lo
	float NdotL = max(dot(l_vecN, l_vecL), 0.0);                
//	Lo += f_r * radiance * NdotL; 
	
	f_diff = f_diff * radiance * NdotL; 
	f_spec = f_spec * radiance * NdotL; 
	if(u_useSimTransmit == 1)
	{
		// Simulate light transmission
		/*vec3 projesCoords = pos_ls.xyz / pos_ls.w;
		projesCoords = projesCoords * 0.5 + 0.5;*/
		vec3 projesCoords = ProjectToScreenUV(pos_ls);
		// depth of the entry point of light
		float depth_in = texture(u_shadowMap, projesCoords.xy).r; 
		// depth of the exit point of light
		float depth_out = projesCoords.z - 0.001; // bias
		// thickness = length of ray inside object
		float thickness = max(depth_out - depth_in, 0.0);
		// color at entry point
		vec3 colIn = vec3(0.0);
		if(u_isLightDir == 0)
		{
			// approximate radiance
			float dist_InToLight = length(pos_world-u_lightPos) / u_distLightMax;
			colIn = u_diffuseColor * 0.5/(dist_InToLight*dist_InToLight);
		}
		else
			colIn = u_diffuseColor; 
		// compute transmission color
		vec3 transmissionColor =  transmittedLight(f_diff, colIn, thickness);
		f_diff = myMax(transmissionColor, f_diff) ;
	}
	Lo = f_diff + f_spec;
	

	// END for each light source

	// Shadow mapping
	float shadow = 0.0;
	if(u_useShadowMap == 1)
	{
		// get shadow factor		
		shadow = ShadowCalculation(pos_ls, l_vecN, l_vecL); 
	}

	// 2.4- Compute ambient --------------------------------------------
	
	// ambient lighting
	vec3 ambient = vec3(0.03) * albedoD;

	// add ambient lighting to color and apply shadow mapping
	color.rgb = ambient + Lo * (1.5 - shadow); // points in shadow still have a 0.5 illumination factor (not complete ambient)

	if(u_useAmbMap == 1) 
	{
		// multiply by ambient occlusion txexture, if any
		color.rgb = ambOcc * color.rgb;
	}

	
	
	if(u_useEnvMapRefrac == 1)
	{
		color.rgb = ambient_refraction(vecN_world, u_camPos - pos_world, u_specularPower, u_cubemap, 7);			
	}
	if(u_useEnvMapReflec == 1)
	{
		color.rgb = ambient_reflection(vecN_world, u_camPos - pos_world, u_specularPower, u_cubemap, 7);			
	}




	//GAMMA CORRECTION
	if(u_useGammaCorrec == 1)
	{
		color.rgb = linear_to_gamma(color.rgb);
	}
	
	frag_color = color;

}