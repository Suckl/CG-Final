#version 330 core
layout(location = 0) in vec3 aPosition;

uniform mat4 uLightSpaceMatrix;
uniform mat4 model;

void main()
{
    gl_Position = uLightSpaceMatrix * model * vec4(aPosition, 1.0f);
}