#version 330 core
layout(location = 0) in vec3 aPosition;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 aTexCoord;

uniform mat4 uModelMatrix;
uniform mat4 uViewMatrix;
uniform mat4 uProjectionMatrix;

out mat4 vWorldToScreen;
out highp vec4 vPosWorld;
out vec2 vTextureCoord;
out vec3 vNormal;


void main(void) {
  vec4 posWorld = uModelMatrix * vec4(aPosition, 1.0);
  vPosWorld = posWorld.xyzw / posWorld.w;

  vWorldToScreen = uProjectionMatrix * uViewMatrix;
  vTextureCoord=aTexCoord;
  vNormal = normalize((uModelMatrix * vec4(aNormal, 0.0)).xyz);

  gl_Position = uProjectionMatrix * uViewMatrix * uModelMatrix * vec4(aPosition, 1.0);
}