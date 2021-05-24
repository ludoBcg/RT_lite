// Vertex shader
//#version 330
//#extension GL_ARB_explicit_attrib_location : require

// VERTEX ATTRIBUTES
//layout(location = 0) in vec4 a_position;
//layout(location = 1) in vec3 a_normal;
//layout(location = 2) in vec3 a_color;
//layout(location = 3) in vec2 a_uv;
//layout(location = 4) in vec3 a_tangent;
//layout(location = 5) in vec3 a_bitangent;


// UNIFORMS
uniform mat4 u_matM;
uniform mat4 u_matV;
uniform mat4 u_matP;
uniform mat4 u_matPV_light; //projection-view matrix of the light camera
uniform vec3 u_camPos;
uniform int u_useTSD;


// OUTPUT
out vec3 vecN_world;
out vec3 vecV_world;
out vec3 vecT_world;
out vec3 vecBT_world;

out vec3 vert_uv;
out vec3 pos_world;

out vec4 pos_ls;




void main()
{
	// compute Model-View and Model-View-Projection matrices
	mat4 matMV = u_matV * u_matM;
	mat4 matMVP = u_matP * matMV;
	
	// vertex position in world space
	pos_world = vec3(u_matM * a_position);
	// View direction vector (in world space)
	vecV_world = normalize( u_camPos - pos_world );
	// Normal vector in world space
	vecN_world = normalize( mat3(u_matM) * a_normal);
// normal matrix = the transpose of the inverse of the upper-left 3x3 part of the model matrix
vecN_world =  normalize( mat3(transpose(inverse(u_matM))) * a_normal );
	// Tangent vector in world space
	vecT_world = normalize(mat3(u_matM) * a_tangent);
	// Bitangent vector in world space
	vecBT_world = normalize(mat3(u_matM) * a_bitangent);
	
	//	Fragment position in light view space
	pos_ls = u_matPV_light * vec4(vec3(a_position.xyz), 1.0);

	// vertex UV
	vert_uv = vec3(a_uv.x, 1.0 - a_uv.y, 0.0);


	gl_Position = matMVP * a_position;
	
	// if texture space diffusion activated
	if(u_useTSD == 1)
	{
		// render to texture coords
		vec2 newUvCoords = a_uv * 2 - 1;
		gl_Position = vec4(newUvCoords, 0, 1.0);
	}

}
