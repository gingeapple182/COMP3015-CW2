#version 460

layout (location = 0) in vec3 VertexPosition;
layout (location = 1) in vec3 VertexNormal;
layout (location = 2) in vec2 VertexTexCoord;
layout (location = 3) in vec4 VertexTangent;

out vec3 SpotDir;
out vec3 LightDir;
out vec3 ViewDir;
out vec2 TexCoord;
out vec3 ViewPos;
out vec4 ShadowCoord;

uniform struct LightInfo{
    vec4 Position;
    vec3 La;
    vec3 L;
}Light;

uniform struct SpotLightInfo{
    vec3 L;
    vec3 La;
    vec3 Position;
    vec3 Direction;
    float Exponent;
    float Cutoff;
}Spot;

uniform mat4 ModelViewMatrix;
uniform mat3 NormalMatrix;
uniform mat4 MVP;
uniform mat4 ShadowMatrix;

void main()
{
    vec3 N = normalize(NormalMatrix * VertexNormal);
    vec3 T = normalize(NormalMatrix * vec3(VertexTangent.xyz));

    T = normalize(T - N * dot(N, T));

    vec3 B = cross(N, T) * VertexTangent.w;

    mat3 TBN = mat3(T, B, N);
    mat3 invTBN = transpose(TBN);

    vec3 Position = (ModelViewMatrix * vec4(VertexPosition, 1.0)).xyz;
    ViewPos = Position;

    vec3 lightDir;
    if (Light.Position.w == 0.0)
        lightDir = normalize(Light.Position.xyz);
    else
        lightDir = Light.Position.xyz - Position;

    LightDir = invTBN * lightDir;
    ViewDir = invTBN * normalize(-Position);
    SpotDir = invTBN * normalize(Spot.Direction);

    TexCoord = VertexTexCoord;

    ShadowCoord = ShadowMatrix * vec4(VertexPosition, 1.0);
    gl_Position = MVP * vec4(VertexPosition, 1.0);
}