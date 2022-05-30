#version 330 core
layout(location = 0) in vec3 aPosition;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 aTexCoord;
out vec3 FragPos;
out vec3 Normal;
out vec2 fTexCoord;
out vec4 fcolor;
uniform vec3 color;
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
void main() {
    fcolor=vec4(color,1.0);
    fTexCoord = aTexCoord;
	FragPos = vec3(model * vec4(aPosition, 1.0f));
	Normal = mat3(transpose(inverse(model))) * aNormal;
	gl_Position = projection * view * model * vec4(aPosition, 1.0f);
}