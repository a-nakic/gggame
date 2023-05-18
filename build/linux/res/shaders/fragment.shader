#version 330 core


in vec3 fragmentColor;
in vec2 UV;
in float fragUseTexture;

out vec4 FragColor;

//layout (binding = 0) 
uniform sampler2D texture_0;

void main ()
{
    vec4 texColor = texture2D (texture_0, UV);

    if (fragUseTexture == 0.0) {
        texColor = vec4 (fragmentColor, texColor.r);
    }

    if (texColor.a < 0.1) {
        discard;
    }

    FragColor = texColor;
}