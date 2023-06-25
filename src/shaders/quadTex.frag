// Fragment shader
#version 330


// ------------------------------------------------------------------------------------------------
// - Render texture to a mesh, with no lighting.
// - Apply Gaussian blur on the texture (horizontal or vertical), if activated.
// This shader is used to blur TSD texture or SSAO map.
//
// A texture mask can be defined by alpha values (1=inside / 0=outside).
// It is used to avoid black texels bleeding during TSD.
// (the texture to blur in TSD is a UV map: we don't want to include texels outside the UV map in 
// the blurring, as it would cause color bleeding along the edges of the UV map).
//
// Based on Texture-Space Diffusion (TSD) implemented by A. Brandstätter:
// 		"Real-time rendering of realistic surfaces using subsurface scattering", Bsc. thesis, 2020.
//		 https://gitlab.com/annabrandy/approx-subsurface-scattering
// ------------------------------------------------------------------------------------------------


// UNIFORMS
uniform sampler2D u_screenTex;
uniform int isBlurOn;
uniform int isFilterH;
uniform int filterSize;

	
// INPUT	
in vec3 vert_uv;


// OUTPUT
out vec4 frag_color;

// GLOBAL VARIABLE
vec3 coefficientSum = vec3(0);




// Performs texture lookup with mask checking:
// Texels outside of mask have 0 alpha.
// Returns texel color if alpha != 0 (inside mask), or returns vec4(0) if alpha == 0 (out of mask).
vec4 texLookup(vec2 uvCoords, float coeff)
{
    vec4 lookup = texture(u_screenTex, uvCoords);
    float maskLookup = lookup.a;
    vec3 color = lookup.xyz; 

    color = clamp(color, 0, 1);
    maskLookup = clamp(maskLookup, 0, 1);

    if (maskLookup > 0)
    {
        coefficientSum += coeff;
 
        return vec4(color * coeff, maskLookup);
    }

    return vec4(0);
}


// MAIN
void main()
{
	// Apply Gaussian Blur if Blur is on
	if(isBlurOn == 1)
	{
		// filterSize = halfwidth = main
		// sigma = m / 1.96 (to cover 96% of the gaussian)
		float m = filterSize;
		float sigma = m / 1.96;

		float weight = 1.0;
		vec4 lookup, avgValue;

		float mapResolution = 2048.0;
		vec2 direction;
		
		// Guassian blur is decomposed into horizontal and vertical weighted sums
		// Horizontal blur if isFilterH is on, Vertical blur if not
		if(isFilterH == 1)
			direction = vec2(1.0f / mapResolution, 0);
		else
			direction = vec2(0, 1.0f/ mapResolution);
				
		
		lookup = texLookup(vert_uv.xy, weight);
		avgValue = lookup;
		

		for (int i = 1; i <= filterSize; i++)
		{
			weight = exp(- 0.5 * i * i / (sigma * sigma));
	 
			avgValue += texLookup(vert_uv.xy + i * direction, weight);
			avgValue += texLookup(vert_uv.xy - i * direction, weight);
		}

		if(avgValue.a != 0)
		{
			vec3 diffuseColor = avgValue.xyz / coefficientSum;
			frag_color = vec4(diffuseColor, lookup.a );
		}
		else
		{
			frag_color = vec4(0.0f);
		}
	
	}
	else
	{
		// final color
		vec4 color = vec4(1.0f);
		color.rgb = texture(u_screenTex, vert_uv.xy).rgb;
		
		frag_color = color;
	}
	
	

}

