#version 460

in vec3 SpotDir;
in vec3 LightDir;
in vec3 ViewDir;
in vec2 TexCoord;
in vec3 ViewPos;

layout (location = 0) out vec4 FragColor;
layout (binding = 0) uniform sampler2D DiffuseTex;
layout (binding = 1) uniform sampler2D NormalMapTex;
layout (binding = 2) uniform sampler2D MixingTex;


uniform struct MaterialInfo{
    vec3 Kd;
    vec3 Ka;
    vec3 Ks;
    float Shininess;
}Material;

uniform struct SpotLightInfo{
    vec3 L;
    vec3 La;
    vec3 Position;
    vec3 Direction;
    float Exponent;
    float Cutoff;
}Spot;

uniform struct LightInfo{
    vec4 Position;
    vec3 La;
    vec3 L;
}Light;

uniform struct FogInfo{
    float MaxDistance;
    float MinDistance;
    vec3 Colour;
}Fog;

//const int levels = 5;
//const float scaleFactor = 1.0/levels;

// Fog controls (simple + robust)
uniform int   FogEnabled;   // 0 = off, 1 = on
uniform float FogScale;     // 1.0 = normal, >1 stronger fog, <1 weaker

// Mixing 
uniform float MixAmount; // 0 = only DiffuseTex, 1 = full MixingTex (scaled by mask)

vec3 getBaseColour()
{
    vec4 a = texture(DiffuseTex, TexCoord);
    vec4 b = texture(MixingTex, TexCoord);

    // Use MixingTex alpha as the per-pixel mask.
    // If your MixingTex doesn't have alpha, use b.r (or dot(b.rgb, vec3(0.333))) instead.
    float mask = b.a;

    float t = clamp(mask * MixAmount, 0.0, 1.0);
    return mix(a.rgb, b.rgb, t);
}

vec3 blinnPhong(vec3 n) {
    vec3 diffuse = vec3(0.0f);
    vec3 spec = vec3(0.0f);

    vec3 texColour = getBaseColour();

    vec3 ambient = Light.La * texColour;
    vec3 s = normalize(LightDir);

    float sDotN = max(dot(s, n), 0.0);
    diffuse = texColour * sDotN;

    if (sDotN > 0.0) {
        vec3 v = normalize(ViewDir);
        vec3 h = normalize(v + s);
        spec = Material.Ks * pow(max(dot(h, n), 0.0), Material.Shininess);
    }
    return ambient + (diffuse + spec) * Light.L;
}

vec3 blinnPhongSpot(vec3 n)
{
    vec3 texColour = getBaseColour();

    vec3 ambient = Spot.La * texColour;

    vec3 s = normalize(LightDir);
    vec3 v = normalize(ViewDir);

    float cosAng = dot(normalize(-s), normalize(SpotDir));

    float outer = cos(Spot.Cutoff);
    float inner = cos(Spot.Cutoff * 0.85);

    float spotEdge = smoothstep(outer, inner, cosAng);
    float spotCore = pow(max(cosAng, 0.0), Spot.Exponent);
    float spotScale = spotEdge * spotCore;

    if (spotScale <= 0.001)
        return ambient;

    float sDotN = max(dot(s, n), 0.0);
    vec3 diffuse = texColour * sDotN;

    vec3 spec = vec3(0.0);
    if (sDotN > 0.0) {
        vec3 h = normalize(v + s);
        spec = Material.Ks * pow(max(dot(h, n), 0.0), Material.Shininess);
    }

    return ambient + spotScale * Spot.L * (diffuse + spec);
}


float fogFactorLinear(float dist)
{
    // 1 = fully shaded, 0 = fully fog
    float f = (Fog.MaxDistance - dist) / (Fog.MaxDistance - Fog.MinDistance);
    return clamp(f, 0.0, 1.0);
}

void main()
{
    vec3 normal = texture(NormalMapTex, TexCoord).xyz;
    normal = normalize(normal * 2.0 - 1.0);

    vec3 litColour = blinnPhongSpot(normal);

    if (FogEnabled != 0)
    {
        float dist = length(ViewPos) * max(FogScale, 0.0001);
        float f = fogFactorLinear(dist);
        vec3 finalColour = mix(Fog.Colour, litColour, f);
        FragColor = vec4(finalColour, 1.0);
    }
    else
    {
        FragColor = vec4(litColour, 1.0);
    }
}
