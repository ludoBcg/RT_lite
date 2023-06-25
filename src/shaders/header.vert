// Vertex header
#version 330

// VERTEX ATTRIBUTES
layout(location = 0) in vec4 a_position;
layout(location = 1) in vec3 a_normal;
layout(location = 2) in vec3 a_color;
layout(location = 3) in vec2 a_uv;
layout(location = 4) in vec3 a_tangent;
layout(location = 5) in vec3 a_bitangent;

const float PI = 3.14159265359;