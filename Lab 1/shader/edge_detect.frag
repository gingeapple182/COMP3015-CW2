#version 460
in vec2 vUV;
layout (location = 0) out vec4 FragColor;

layout (binding = 0) uniform sampler2D MaskTex;

uniform vec2  TexelSize;   // (1/width, 1/height)
uniform float Threshold;   // edge cutoff
uniform float Opacity;     // 0..1
uniform float Thickness;   // 1..N
uniform vec3  EdgeColour;  // outline colour

float sampleMask(vec2 uv)
{
    return texture(MaskTex, uv).r; // 0..1 mask
}

void main()
{
    vec2 t = TexelSize * max(Thickness, 0.0001);

    // Sobel kernel on mask intensity
    float tl = sampleMask(vUV + vec2(-t.x,  t.y));
    float  l = sampleMask(vUV + vec2(-t.x,  0.0));
    float bl = sampleMask(vUV + vec2(-t.x, -t.y));

    float  t0 = sampleMask(vUV + vec2( 0.0,  t.y));
    float  b0 = sampleMask(vUV + vec2( 0.0, -t.y));

    float tr = sampleMask(vUV + vec2( t.x,  t.y));
    float  r = sampleMask(vUV + vec2( t.x,  0.0));
    float br = sampleMask(vUV + vec2( t.x, -t.y));

    float gx = (-1.0*tl) + (1.0*tr)
             + (-2.0*l ) + (2.0*r )
             + (-1.0*bl) + (1.0*br);

    float gy = ( 1.0*tl) + (2.0*t0) + (1.0*tr)
             + (-1.0*bl) + (-2.0*b0) + (-1.0*br);

    float g = sqrt(gx*gx + gy*gy);

    // threshold -> alpha
    float a = smoothstep(Threshold, Threshold * 1.5, g) * Opacity;

    if (a <= 0.001)
        discard;

    FragColor = vec4(EdgeColour, a);
}