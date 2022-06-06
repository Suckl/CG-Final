
#version 330 core

// Pbr related variables
uniform vec3 uCameraPos;
uniform vec3 uLightPos;

uniform vec3 uLightRadiance;
uniform float ka;

uniform sampler2D uAlbedoMap;
uniform float uMetallic;
uniform float uRoughness;
uniform vec3 uColor;

in vec2 vTextureCoord;
in vec3 vFragPos;
in vec3 vNormal;

uniform samplerCube uShadowMap;
uniform float far_plane;

#define PI 3.141592653589793

// PBR part
float DistributionGGX(vec3 N, vec3 H, float roughness)
{
    float a2 = pow(roughness, 4.0);
    float NdotH = clamp(dot(N, H), 0.0, 1.0);
    float d = NdotH * NdotH * (a2 - 1.0) + 1.0;
    return a2 / (PI * d * d);
}

float GeometrySchlickGGX(float NdotV, float roughness)
{
    NdotV = clamp(NdotV, 0.0, 1.0);
    float k = (roughness + 1.0) * (roughness + 1.0) / 8.0;    
    return NdotV / (NdotV * (1.0 - k) + k);
}

float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness)
{
    return GeometrySchlickGGX(dot(L, N), roughness) * GeometrySchlickGGX(dot(V, N), roughness);
}

vec3 fresnelSchlick(vec3 F0, vec3 V, vec3 H)
{
    float theta = clamp(dot(V, H), 0.0, 1.0);
    return F0 + (1.0 - F0) * pow(1.0 - theta, 5.0);
}

vec3 PBRcolor()
{
    vec3 uLightDir = normalize(uLightPos - vFragPos);

    vec3 albedo = pow(texture2D(uAlbedoMap, vTextureCoord).rgb, vec3(2.2));
    if(albedo==vec3(0.0)) albedo=uColor;
    vec3 N = normalize(vNormal);
    vec3 V = normalize(uCameraPos - vFragPos);
    float NdotV = max(dot(N, V), 0.0);
    vec3 F0 = vec3(0.04); 
    F0 = mix(F0, albedo,uMetallic);

    vec3 Lo = vec3(0.0);

    vec3 L = normalize(uLightDir);
    vec3 H = normalize(V + L);
    float NdotL = max(dot(N, L), 0.0); 

    vec3 radiance = uLightRadiance;

    float NDF = DistributionGGX(N, H, uRoughness);   
    float G   = GeometrySmith(N, V, L, uRoughness); 
    vec3 F = fresnelSchlick(F0, V, H);

    vec3 numerator    = NDF * G * F; 
    float denominator = max((4.0 * NdotL * NdotV), 0.001);
    vec3 BRDF = numerator / denominator;

    Lo += BRDF * radiance * NdotL;
    vec3 color = Lo + vec3(0.001) * texture2D(uAlbedoMap, vTextureCoord).rgb;

    color = color / (color + vec3(1.0));
    color = pow(color, vec3(1.0/2.2)); 
    return color;
}

// Shadow part
#define BIAS_SIZE 0.00233

float Bias(){
    vec3 uLightDir = normalize(uLightPos - vFragPos);
    float bias = max(BIAS_SIZE * (1.0 - dot(vNormal, uLightDir)), BIAS_SIZE);
    return  bias;
}

float ShadowCalculation(vec3 fragPos)
{
    // Get vector between fragment position and light position
    vec3 fragToLight = fragPos - uLightPos;
    // Use the fragment to light vector to sample from the depth map    
    float closestDepth = texture(uShadowMap, fragToLight).r;
    // It is currently in linear range between [0,1]. Let's re-transform it back to original depth value
    closestDepth *= far_plane;
    // Now get current linear depth as the length between the fragment and light position
    float currentDepth = length(fragToLight);
    // Now test for shadows
    float bias = Bias(); // We use a much larger bias since depth is now in [near_plane, far_plane] range
    float shadow = currentDepth -  bias > closestDepth ? 1.0 : 0.0;

    return shadow;
}

void main()
{           
    vec3 ambient = ka * vec3(0.3);

    float shadow = ShadowCalculation(vFragPos);  

    vec3 color = PBRcolor() * (1.0 - shadow) + ambient;
    gl_FragColor = vec4(color, 1.0);

    /////////////////////////////////////////////////////////////////////////////////

    // vec3 color = vec3(0.6);
    // vec3 normal = normalize(vNormal);
    // vec3 lightColor = vec3(0.3);

    // // Ambient
    // vec3 ambient = 0.3 * color;

    // // Diffuse
    // vec3 lightDir = normalize(uLightPos - vFragPos);
    // float diff = max(dot(lightDir, normal), 0.0);
    // vec3 diffuse = diff * lightColor;

    // // Specular
    // vec3 viewDir = normalize(uCameraPos - vFragPos);
    // vec3 reflectDir = reflect(-lightDir, normal);
    // float spec = 0.0;
    // vec3 halfwayDir = normalize(lightDir + viewDir);  
    // spec = pow(max(dot(normal, halfwayDir), 0.0), 64.0);
    // vec3 specular = spec * lightColor;    

    // // Calculate shadow
    // float shadow = ShadowCalculation(vFragPos);

    // color = (ambient + (1.0 - shadow) * (diffuse + specular)) * color;   
    // gl_FragColor = vec4(color, 1.0);
}