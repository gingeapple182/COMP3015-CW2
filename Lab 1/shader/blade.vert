#version 460

layout (location = 0) in vec3 VertexPosition;
layout (location = 1) in vec3 VertexNormal;

out vec3 vNormalVS;
out vec3 vPosVS;

uniform mat4 ModelViewMatrix;
uniform mat3 NormalMatrix;
uniform mat4 MVP;

void main()
{
	vNormalVS = normalize(NormalMatrix * VertexNormal);
	vPosVS = (ModelViewMatrix * vec4(VertexPosition, 1.0)).xyz;

	gl_Position = MVP * vec4(VertexPosition, 1.0);
}