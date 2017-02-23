// VERTEX SHADER 
#version 410 

layout(location=0) in vec4 position;
layout(location=1) in vec4 normal;
layout(location=2) in vec4 tangent;
layout(location=3) in vec2 texcoord;
layout(location=4) in vec4 weights;
layout(location=5) in vec4 indices;

out vec3 vNormal;
out vec3 vPosition;
out vec2 uv;

out vec3 debug;

uniform mat4 pvmMatrix;
uniform mat4 modelMatrix;

void main() {
	// Homogenise Position and Normal
	vec4 hP = vec4(position.xyz, 1);
	vec4 hN = vec4(normal.xyz, 0);

	// Apply Transforms
	vPosition = (modelMatrix * hP).xyz;
	vNormal = (inverse(transpose(modelMatrix)) * hN).xyz; 
	//vNormal = (modelMatrix * hN).xyz; 

	uv = texcoord;

	// Set gl Position
	gl_Position = pvmMatrix * hP; 
}