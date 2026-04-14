#version 460

in vec3 vNormalVS;
in vec3 vPosVS;

layout (location = 0) out vec4 FragColor;

// Set from C++
uniform vec3 BladeColour;     
uniform float Intensity;      
uniform float RimPower;       
uniform float RimIntensity;   

// New (for glow shell control)
uniform float Alpha;          // overall alpha scale, e.g. 0.15..0.6
uniform float CoreAlpha;      // base alpha even at centre, e.g. 0.0..0.2

void main()
{
	vec3 N = normalize(vNormalVS);
	vec3 V = normalize(-vPosVS); // camera at origin in view-space

	float ndv = max(dot(N, V), 0.0);
	float rim = pow(1.0 - ndv, RimPower);

	// Emissive colour (core + extra rim pop)
	vec3 col = BladeColour * Intensity;
	col += BladeColour * (rim * RimIntensity * Intensity);

	// Alpha: mostly rim-driven for glow; CoreAlpha lets you keep a faint fill
	float a = clamp(CoreAlpha + rim * Alpha, 0.0, 1.0);

	FragColor = vec4(col, a);
}