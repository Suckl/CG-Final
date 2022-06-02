#version 330 core
uniform vec3 uCameraPos;
uniform vec3 uLightRadiance;
uniform vec3 uLightDir;

uniform sampler2D uAlbedoMap;
uniform float uMetallic;
uniform float uRoughness;
uniform vec3 uColor;

uniform sampler2D uShadowMap;

in vec2 vTextureCoord;
in vec3 vFragPos;
in vec3 vNormal;
in vec4 vPositionFromLight;

const float PI = 3.14159265359;

// Shadow map related variables
#define NUM_SAMPLES 20
#define BLOCKER_SEARCH_NUM_SAMPLES NUM_SAMPLES
#define PCF_NUM_SAMPLES NUM_SAMPLES
#define NUM_RINGS 10

#define EPS 1e-3
#define PI 3.141592653589793
#define PI2 6.283185307179586

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
    vec3 color = Lo+vec3(0.001)*texture2D(uAlbedoMap, vTextureCoord).rgb;

    color = color / (color + vec3(1.0));
    color = pow(color, vec3(1.0/2.2)); 
    return color;
}

// Shadow part
float unpack(vec4 rgbaDepth) {
    const vec4 bitShift = vec4(1.0, 1.0/256.0, 1.0/(256.0*256.0), 1.0/(256.0*256.0*256.0));
    return dot(rgbaDepth, bitShift);
}

float useShadowMap(sampler2D shadowMap, vec4 shadowCoord){
  // 光方向的最小深度
  vec4 closestDepthVec = texture2D(shadowMap, shadowCoord.xy); 
  float closestDepth = unpack(closestDepthVec);
  // 该texel在光方向的深度
  float currentDepth = shadowCoord.z;
  // 是否是最小深度？
  float shadow = closestDepth > currentDepth ? 1.0 : 0.0;
  return shadow;
}

void main(void) {
    float visibility;
    vec3 shadowCoord = vPositionFromLight.xyz / vPositionFromLight.w;
    shadowCoord = shadowCoord * 0.5 + 0.5;
    visibility = useShadowMap(uShadowMap, vec4(shadowCoord, 1.0));
    //visibility = PCF(uShadowMap, vec4(shadowCoord, 1.0));
    
    vec3 color = PBRcolor();
    gl_FragColor = vec4(color, 1.0);
}