#version 330 core
layout(location = 0) in vec3 aPosition;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 aTexCoord;

out vec2 vTextureCoord;
out vec3 vFragPos;
out vec3 vNormal;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

// uniform bool reverse_normals;

void main()
{
    gl_Position = projection * view * model * vec4(aPosition, 1.0f);
    vFragPos = vec3(model * vec4(aPosition, 1.0));
    // if(reverse_normals) // A slight hack to make sure the outer large cube displays lighting from the 'inside' instead of the default 'outside'.
    //     vNormal = transpose(inverse(mat3(model))) * (-1.0 * aNormal);
    // else
        vNormal = transpose(inverse(mat3(model))) * aNormal;
    vTextureCoord = aTexCoord;
}