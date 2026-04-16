#version 460 core

in vec2 TexCoord;
out vec4 FragColor;

uniform sampler2D FontTex;
uniform vec3 TextColour;

void main()
{
    vec4 tex = texture(FontTex, TexCoord);

    if (tex.a < 0.1)
        discard;

    FragColor = vec4(TextColour, tex.a);
}