#version 330 core

layout (location = 0) in vec3 vertexPosition;
layout (location = 1) in vec3 vertexColor;
layout (location = 2) in vec2 vertexUV;
layout (location = 3) in float useTexture;

out vec3 fragmentColor;
out vec2 UV;
out float fragUseTexture;

uniform mat4 MVP;

void main ()
{
    //gl_Position = vec4 (vertexPosition.xy, 0.0, 1.0);
    gl_Position = MVP * vec4 (vertexPosition, 1.0);

    UV = vertexUV;
    fragmentColor = vertexColor;
    fragUseTexture = useTexture;
}