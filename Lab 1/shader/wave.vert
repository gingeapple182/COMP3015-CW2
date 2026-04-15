#version 460 core

layout (location = 0) in vec3 VertexPosition;
layout (location = 1) in vec3 VertexNormal;
layout (location = 2) in vec2 VertexTexCoord;

out vec4 Position;
out vec3 Normal;
out vec2 TexCoord;
out vec4 ShadowCoord;
out vec3 WorldPos;

uniform float Time;
uniform float ForwardOffset;
uniform float FlowStrength = 3.5;

// Wave parameters
uniform float Amp1 = 0.30;
uniform float Amp2 = 0.07;
uniform float Amp3 = 0.05;

uniform float Freq1 = 2.0;
uniform float Freq2 = 3.1;
uniform float Freq3 = 1.5;

uniform float Speed1 = 2.5;
uniform float Speed2 = 0.8;
uniform float Speed3 = 1.7;

// Wave directions
const vec2 Dir1 = vec2(1.0, 0.3);
const vec2 Dir2 = vec2(-0.4, 1.0);
const vec2 Dir3 = vec2(0.7, -0.6);

uniform mat4 ModelViewMatrix;
uniform mat3 NormalMatrix;
uniform mat4 MVP;
uniform mat4 ShadowMatrix;
uniform mat4 ModelMatrix;



void main()
{
    vec4 pos = vec4(VertexPosition, 1.0);

    vec2 d1 = normalize(Dir1);
    vec2 d2 = normalize(Dir2);
    vec2 d3 = normalize(Dir3);

    vec2 flowPos = pos.xz + vec2(0.0, ForwardOffset * FlowStrength);

    float w1 = dot(d1, flowPos) * Freq1 + Time * Speed1;
    float w2 = dot(d2, flowPos) * Freq2 + Time * Speed2;
    float w3 = dot(d3, flowPos) * Freq3 + Time * Speed3;

    pos.y = Amp1 * sin(w1)
          + Amp2 * sin(w2)
          + Amp3 * sin(w3);

    vec4 worldPos = ModelMatrix * pos;
    WorldPos = worldPos.xyz;

    float dW1dx = Amp1 * Freq1 * cos(w1) * d1.x;
    float dW1dz = Amp1 * Freq1 * cos(w1) * d1.y;

    float dW2dx = Amp2 * Freq2 * cos(w2) * d2.x;
    float dW2dz = Amp2 * Freq2 * cos(w2) * d2.y;

    float dW3dx = Amp3 * Freq3 * cos(w3) * d3.x;
    float dW3dz = Amp3 * Freq3 * cos(w3) * d3.y;

    float dX = dW1dx + dW2dx + dW3dx;
    float dZ = dW1dz + dW2dz + dW3dz;

    vec3 n = normalize(vec3(-dX, 1.0, -dZ));

    Position = ModelViewMatrix * pos;
    Normal = normalize(NormalMatrix * n);
    //TexCoord = VertexTexCoord + vec2(Time * 0.03, Time * 0.01);;
    //TexCoord = worldPos.xz * 0.08 + vec2(Time * 0.03, Time * 0.01);
    TexCoord = worldPos.xz * 0.08 + vec2(0.0, -ForwardOffset);
    ShadowCoord = ShadowMatrix * pos;

    gl_Position = MVP * pos;
}