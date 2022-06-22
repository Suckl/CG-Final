#version 330 core
layout(location = 0) in vec2 aPosition;
layout(location = 2) in vec2 aTexCoords;

uniform mat4 uViewMatrix;
uniform mat4 uProjectionMatrix;
out mat4 vWorldToScreen;


void main(void) {
  vWorldToScreen = uProjectionMatrix * uViewMatrix;
  gl_Position = vec4(aPosition, 0.0f, 1.0f);
}