#version 460 core

in vec3 Normal;
in vec4 Position;
in vec2 TexCoord;
in vec4 ShadowCoord;

out vec4 FragColor;

layout(binding = 0) uniform sampler2D DiffuseTex;
layout(binding = 3) uniform sampler2DShadow ShadowMap;

uniform vec4 LightPosition;
uniform vec3 LightIntensity;
uniform vec3 AmbientLight;

uniform vec3 MaterialKd;
uniform vec3 MaterialKa;
uniform vec3 MaterialKs;
uniform float MaterialShininess;

float getShadowFactor()
{
    if (ShadowCoord.w <= 0.0)
        return 1.0;

    vec3 projCoords = ShadowCoord.xyz / ShadowCoord.w;

    if (projCoords.x < 0.0 || projCoords.x > 1.0 ||
        projCoords.y < 0.0 || projCoords.y > 1.0 ||
        projCoords.z < 0.0 || projCoords.z > 1.0)
        return 1.0;

    return texture(ShadowMap, projCoords);
}

void main()
{
    vec3 n = normalize(Normal);

    vec3 s;
    if (LightPosition.w == 0.0)
        s = normalize(LightPosition.xyz);
    else
        s = normalize(LightPosition.xyz - Position.xyz);

    vec3 v = normalize(-Position.xyz);
    vec3 r = reflect(-s, n);

    float sDotN = max(dot(s, n), 0.0);

    vec3 diffuse = MaterialKd * sDotN;
    vec3 spec = vec3(0.0);

    if (sDotN > 0.0)
        spec = MaterialKs * pow(max(dot(r, v), 0.0), MaterialShininess);

    vec4 texColor = texture(DiffuseTex, TexCoord);

    float shadow = getShadowFactor();

    vec3 ambient = AmbientLight * MaterialKa;
    vec3 lit = ambient + (diffuse + spec) * LightIntensity * shadow;

    FragColor = vec4(lit, 1.0) * texColor;
}