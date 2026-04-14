#version 460 core

in vec3 Normal;
in vec4 Position;
in vec2 TexCoord;

out vec4 FragColor;

uniform sampler2D DiffuseTex;

uniform vec3 LightIntensity;
uniform vec3 MaterialKd;
uniform vec3 MaterialKa;
uniform vec3 MaterialKs;
uniform float MaterialShininess;

void main()
{
    vec3 n = normalize(Normal);
    vec3 s = normalize(vec3(0,1,0) - Position.xyz);
    vec3 v = normalize(-Position.xyz);
    vec3 r = reflect(-s, n);

    float sDotN = max(dot(s,n), 0.0);

    vec3 diffuse = MaterialKd * sDotN;
    vec3 spec = vec3(0.0);

    if(sDotN > 0.0)
        spec = MaterialKs * pow(max(dot(r,v),0.0), MaterialShininess);

    vec4 texColor = texture(DiffuseTex, TexCoord);

    FragColor = vec4(MaterialKa + diffuse + spec, 1.0) * texColor;
}
