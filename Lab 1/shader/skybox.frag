#version 460

layout (binding = 0) uniform samplerCube SkyboxTex;

layout (location = 0) out vec4 FragColour;

in vec3 Vec;

// Fog controls
uniform int  FogEnabled;     // 0/1
uniform vec3 FogColour;      // same as your scene fog colour
uniform float SkyFogAmount;  // 0..1 (how much haze over the skybox)

void main() {
    vec3 texColour = texture(SkyboxTex, normalize(Vec)).rgb;

    if (FogEnabled != 0) {
        texColour = mix(texColour, FogColour, clamp(SkyFogAmount, 0.0, 1.0));
    }

    FragColour = vec4(texColour, 1.0);
}