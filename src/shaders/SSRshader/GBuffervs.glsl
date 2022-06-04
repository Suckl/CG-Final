#version 330 core
layout(location = 0) in vec3 aPosition;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 aTexCoord;

uniform mat4 uLightVP;
uniform mat4 uModelMatrix;
uniform mat4 uViewMatrix;
uniform mat4 uProjectionMatrix;

out mat4 vWorldToLight;

out vec2 vTextureCoord;
out vec3 vNormalWorld;
out vec4 vPosWorld;
out float vDepth;

void main(void) {
  vec4 posWorld = uModelMatrix * vec4(aPosition, 1.0);
  vPosWorld = posWorld.xyzw / posWorld.w;
  vec4 normalWorld = uModelMatrix * vec4(aNormal, 0.0);
  vNormalWorld = normalize(normalWorld.xyz);
  vTextureCoord = aTexCoord;
  vWorldToLight = uLightVP;

  gl_Position = uProjectionMatrix * uViewMatrix * uModelMatrix * vec4(aPosition, 1.0);
  vDepth = gl_Position.w;
}