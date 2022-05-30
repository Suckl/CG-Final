#version 330 core
layout(location = 0) in vec3 aPosition;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 aTexCoord;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

out vec2 vTextureCoord;
out vec3 vFragPos;
out vec3 vNormal;


void main(void) {

  vFragPos = (model * vec4(aPosition, 1.0)).xyz;
  vNormal = (model * vec4(aNormal, 0.0)).xyz;

  gl_Position = projection * view * model * vec4(aPosition, 1.0);

  vTextureCoord = aTexCoord;

}