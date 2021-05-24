// Fragment header
#version 330

// UNIFORMS
uniform mat4 u_matM;
uniform samplerCube u_cubemap;
uniform sampler2D u_shadowMap;


const float PI = 3.14159265359;

vec3 ambient_reflection(in vec3 _N, in vec3 _V, in float _specularPower, in samplerCube _cubemap, in float _maxLevel)
{
    vec3 R = reflect(-_V, _N);
	R = ( inverse( u_matM ) * vec4(R.xyz, 1.0f) ).xyz;
    float gloss = log2(_specularPower) / 10.0;
    float level = clamp(1.0 - gloss, 0.0, 1.0) * _maxLevel;
	return texture(_cubemap, R).rgb;

}

vec3 ambient_refraction(in vec3 _N, in vec3 _V, in float _specularPower, in samplerCube _cubemap, in float _maxLevel)
{
	float ratio = 1.00 / 1.52;
    vec3 R = refract(-_V, _N, ratio);
	R = ( inverse(u_matM ) * vec4(R.xyz, 1.0f) ).xyz;	
	return texture(_cubemap, R).rgb;

}


// Normal distribution function (D)
float DistributionGGX(vec3 _N, vec3 _H, float _a)
{
	// Compute NDF using Trowbridge-Reitz GGX
	// (statistically approximate the general alignment of the microfacets given some roughness parameter)
	
	float a2     = _a*_a;
	float NdotH  = max(dot(_N, _H), 0.0);
	float NdotH2 = NdotH*NdotH;

	float nom    = a2;
	float denom  = (NdotH2 * (a2 - 1.0) + 1.0);
	denom        = PI * denom * denom;

	return nom / denom;
}

// Geometry function (G)
float GeometrySchlickGGX(float _NdotV, float _k)
{
    float nom   = _NdotV;
    float denom = _NdotV * (1.0 - _k) + _k;
	
    return nom / denom;
}
  
float GeometrySmith(vec3 _N, vec3 _V, vec3 _L, float _k)
{
	// Compute geometry function using Schlick-GGX
	// (statistically approximates the relative surface area where its micro surface-details overshadow each other causing light rays to be occluded)

    float NdotV = max(dot(_N, _V), 0.0);
    float NdotL = max(dot(_N, _L), 0.0);
    float ggx1 = GeometrySchlickGGX(NdotV, _k);
    float ggx2 = GeometrySchlickGGX(NdotL, _k);
	
    return ggx1 * ggx2;
}

// Fresnel equation (F)
vec3 fresnelSchlick(float _cosTheta, vec3 _F0)
{
	// Fresnel-Schlick approximation for F
	// (describes the ratio of light that gets reflected over the light that gets refracted)
	
    return _F0 + (1.0 - _F0) * pow(1.0 - _cosTheta, 5.0);
}

float ShadowCalculation(vec4 pos_ls, vec3 normal, vec3 lightDir)
{
    // perform perspective divide
    vec3 projCoords = pos_ls.xyz / pos_ls.w;
    // transform to [0,1] range
    projCoords = projCoords * 0.5 + 0.5;
    // get closest depth value from light's perspective (using [0,1] range fragPosLight as coords)
    float closestDepth = texture(u_shadowMap, projCoords.xy).r; 


	
    // get depth of current fragment from light's perspective
    float currentDepth = projCoords.z;
    // check whether current frag pos is in shadow
//    float shadow = currentDepth > closestDepth  ? 1.0 : 0.0;
float maxBias = 0.005;
float minBias = 0.0001;


	float bias = max(maxBias * (1.0 - dot(normal, lightDir)), minBias);  

	
	// percentage-closer filtering, to avoid shadow aliasing	
	float shadow = 0.0;
	vec2 texelSize = 1.0 / textureSize(u_shadowMap, 0);
	for(int x = -1; x <= 1; ++x)
	{
		for(int y = -1; y <= 1; ++y)
		{
			float pcfDepth = texture(u_shadowMap, projCoords.xy + vec2(x, y) * texelSize).r; 


			shadow += currentDepth - bias > pcfDepth ? 1.0 : 0.0;        
		}    
	}
	shadow /= 9.0;

// deactivate since we use pcf
//	float shadow = currentDepth - bias > closestDepth  ? 1.0 : 0.0;  


    return shadow;

}  

vec3 linear_to_gamma(in vec3 _color)
{
    return pow(_color, vec3(1.0f / 2.2f));
}

// Projects a position from view space to according texture coordinates in screen space
vec3 ProjectToScreenUV(vec4 pos_lightSpace) 
{
	vec3 projesCoords = pos_lightSpace.xyz / pos_lightSpace.w;
	return projesCoords * 0.5 + 0.5;
}


/** Maps the given thickness to a certain transmission value according to 
    the strictly decreasing absorption function.
    
    Supported absorption functions:
    - 0: exonential
    - 1: linear
    - 2: quadratic
    - 3: cubic
    - 4: sqare root
    - 5: cube root
*/
vec3 getTransmittedFactor(float thickness)
{
int absorptionFunction = 0;
vec3 absorptionParameter = vec3(1.0, 1.0, 1.0);

    vec3 x = absorptionParameter * thickness;
    vec3 t = vec3(0);

    switch (absorptionFunction)
    {
        case 0: t = exp (- 5 * x); break;
        case 1: t = 1 - x; break; 
        case 2: t = 1 - pow(x, vec3(2)); break; 
        case 3: t = 1 - pow(x, vec3(3)); break;
        case 4: t = 1 - sqrt(x); break;
        case 5: t = 1 - pow(x, vec3(1/3.0)); break;
        case 6: t = exp(- 5 * pow(x, vec3(2)));
    }
    return clamp(t, 0, 1);
}


/** Evaluates the transmitted term. It contains a linear interpolation 
    between information on this and on the other side. */
vec3 transmittedLight(vec3 col_out, vec3 col_in, float thickness)
{
	vec3 transmitColor = vec3(1.0, 1.0, 1.0); // color let passed through by material
//	vec3 objectColor = col_in;
	float alpha1 = 0.25;
//float alpha3 = 0.05; alpha3 = 0.0;
    /*if (m.kt <= 0)
        return vec3(0);*/
    
	// transmission factor
    vec3 vec_td = getTransmittedFactor(thickness);
    float td = (vec_td.x + vec_td.y + vec_td.z) / 3.0f;

	// weighted avg between colors at entry and exit point
    vec3 mixedCol =  mix(col_out, col_in, alpha1 * td );

    vec3 transmissionColor = mixedCol; //mix(mixedCol, (1 - td) * transmitColor, alpha3);

	//return transmissionColor;
	float kt = 0.2;
	return max(kt * td * transmissionColor, vec3(0));
}

/** Returns component-wise maximum of two vectors */
vec3 myMax(vec3 a, vec3 b)
{
    a = clamp(a, 0, 1);
    b = clamp(b, 0, 1);
    return vec3(max(a.x, b.x), max(a.y, b.y), max(a.z, b.z));
}