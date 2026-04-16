#version 460

layout (location = 0) in vec3 VertexInitVel;
layout (location = 1) in float VertexBirthTime;

out float Transp;
out vec2 TexCoord;

uniform float Time;
uniform vec3 Gravity = vec3(0.0, 0.15, 0.0);
uniform float ParticleLifetime = 2.5;
uniform float MinParticleSize = 0.25;
uniform float MaxParticleSize = 1.8;
uniform vec3 EmitterPos;

uniform mat4 MV;
uniform mat4 Proj;

const vec3 offsets[6] = vec3[](
	vec3(-0.5, -0.5, 0.0),
	vec3( 0.5, -0.5, 0.0),
	vec3( 0.5,  0.5, 0.0),
	vec3(-0.5, -0.5, 0.0),
	vec3( 0.5,  0.5, 0.0),
	vec3(-0.5,  0.5, 0.0)
);

const vec2 texCoords[6] = vec2[](
	vec2(0.0, 0.0),
	vec2(1.0, 0.0),
	vec2(1.0, 1.0),
	vec2(0.0, 0.0),
	vec2(1.0, 1.0),
	vec2(0.0, 1.0)
);

void main()
{
	vec3 cameraPos = vec3(0.0);
	Transp = 0.0;

	float t = Time - VertexBirthTime;

	if (t >= 0.0)
	{
		t = mod(t, ParticleLifetime);
		float agePct = t / ParticleLifetime;

		vec3 worldPos = EmitterPos + VertexInitVel * t + Gravity * t * t;

		cameraPos = (MV * vec4(worldPos, 1.0)).xyz;
		cameraPos += offsets[gl_VertexID] * mix(MinParticleSize, MaxParticleSize, agePct);

		Transp = clamp(1.0 - agePct, 0.0, 1.0);
	}

	TexCoord = texCoords[gl_VertexID];
	gl_Position = Proj * vec4(cameraPos, 1.0);
}