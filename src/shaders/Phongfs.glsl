#version 330 core

in vec3 FragPos;
in vec3 Normal;

out vec4 color;

in vec2 fTexCoord;
in vec4 fcolor;

uniform vec3 viewPos;
uniform vec3 lightPos;
uniform sampler2D mapKd;

void main(){
    vec4 ambient = vec4(0.1f);
    vec3 lightdir = lightPos-FragPos;
    lightdir = normalize(lightdir);

    vec3 viewdir = normalize(viewPos - FragPos);
    color = fcolor * texture(mapKd, fTexCoord);
    if( color.xyz == vec3(0.0) ) color = fcolor;
    vec4 diffuse = max(dot(lightdir, Normal), 0.0f) * color;
    vec3 halfvec = normalize(lightdir + viewdir);
    vec3 specular = pow(max(dot(halfvec, Normal), 0.0f), 4) * vec3(0.05);
    color=ambient + diffuse + vec4(specular,1.0);
}