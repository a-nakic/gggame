#version 330 core


in vec3 fragmentColor;
in vec2 UV;
in float fragUseTexture;

out vec4 FragColor;

uniform sampler2D texture_0;

void main ()
{
    vec4 texColor;

    if (fragUseTexture == 1.0) 
        //FragColor = texture (texture_0, UV);
        texColor = texture2D (texture_0, UV);
    else
        texColor = vec4 (fragmentColor, 0.3f);

    if (texColor.a < 0.1) {
        discard;
    }

    FragColor = texColor;
}