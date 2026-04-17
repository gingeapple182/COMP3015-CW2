#version 460

layout (location = 0) in vec3 VertexInitVel;
layout (location = 1) in float VertexBirthTime;

out float Transp;
out vec2 TexCoord;

uniform float Time;
uniform vec3 Gravity = vec3(0.0, -0.05, 0.0);
uniform float ParticleLifetime;
uniform float ParticleSize = 1.0;
uniform vec3 EmitterPos;


uniform mat4 MV;
uniform mat4 Proj;

const vec3 offsets[] = vec3[](
    vec3(-0.5, -0.5, 0),
    vec3( 0.5, -0.5, 0),
    vec3( 0.5,  0.5, 0),
    vec3(-0.5, -0.5, 0),
    vec3( 0.5,  0.5, 0),
    vec3(-0.5,  0.5, 0)
);

const vec2 texCoords[] = vec2[](
    vec2(0,0),
    vec2(1,0),
    vec2(1,1),
    vec2(0,0),
    vec2(1,1),
    vec2(0,1)
);

void main()
{
    //float t = Time - VertexBirthTime;
    float t = mod(Time - VertexBirthTime, ParticleLifetime); // Looping particles
    vec3 pos = EmitterPos + VertexInitVel * t + Gravity * t * t;
    vec3 cameraPos = (MV * vec4(pos, 1.0)).xyz + offsets[gl_VertexID] * ParticleSize;

    Transp = 1.0 - (t / ParticleLifetime);

    TexCoord = texCoords[gl_VertexID];
    gl_Position = Proj * vec4(cameraPos, 1.0);
}
