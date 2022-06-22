#version 330 core

// Pbr related variables
uniform vec3 uCameraPos;
uniform vec3 uLightRadiance;
uniform vec3 uLightPos;
uniform float ka;

uniform sampler2D uShadowMap;
uniform sampler2D uAlbedoMap;
uniform float uMetallic;
uniform float uRoughness;
uniform vec3 uColor;

in vec2 vTextureCoord;
in vec3 vFragPos;
in vec3 vNormal;

// Shadow map related variables
#define NUM_SAMPLES 50
#define BLOCKER_SEARCH_NUM_SAMPLES NUM_SAMPLES
#define PCF_NUM_SAMPLES NUM_SAMPLES
#define NUM_RINGS 10

#define EPS 1e-3
#define PI 3.141592653589793
#define PI2 6.283185307179586


in vec4 vPositionFromLight;

highp float rand_1to1(highp float x ) { 
  // -1 -1
  return fract(sin(x)*10000.0);
}

highp float rand_2to1(vec2 uv ) { 
  // 0 - 1
	const highp float a = 12.9898, b = 78.233, c = 43758.5453;
	highp float dt = dot( uv.xy, vec2( a,b ) ), sn = mod( dt, PI );
	return fract(sin(sn) * c);
}

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
    vec3 uLightDir = normalize(uLightPos);

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
float unpack(vec4 rgbaDepth) {
    const vec4 bitShift = vec4(1.0, 1.0/256.0, 1.0/(256.0*256.0), 1.0/(256.0*256.0*256.0));
    float depth =dot(rgbaDepth, bitShift) ;
    //shadow map 没有深度值的地方默认是0 导致的有噪点
    if(abs(depth)<EPS){
      depth=1.0;
    }

    return  depth;
}

#define BIAS_SIZE 0.00233

float Bias(){
    vec3 uLightDir = normalize(uLightPos);
    float bias = max(BIAS_SIZE * (1.0 - dot(vNormal, uLightDir)), BIAS_SIZE);
    return  bias;
}

vec2 poissonDisk[NUM_SAMPLES];

void poissonDiskSamples( const in vec2 randomSeed ) {

    float ANGLE_STEP = PI2 * float( NUM_RINGS ) / float( NUM_SAMPLES );
    float INV_NUM_SAMPLES = 1.0 / float( NUM_SAMPLES );

    float angle = rand_2to1( randomSeed ) * PI2;
    float radius = INV_NUM_SAMPLES;
    float radiusStep = radius;

    for( int i = 0; i < NUM_SAMPLES; i ++ ) {
    poissonDisk[i] = vec2( cos( angle ), sin( angle ) ) * pow( radius, 0.75 );
    radius += radiusStep;
    angle += ANGLE_STEP;
    }
}

float PCF(sampler2D shadowMap, vec4 coords) {
    // 采样
    poissonDiskSamples(coords.xy);
    //uniformDiskSamples(coords.xy);

    // shadow map 的大小, 越大滤波的范围越小
    float textureSize = 2048.0;
    // 滤波的步长
    float filterStride = 5.0;
    // 滤波窗口的范围
    float filterRange = 1.0 / textureSize * filterStride;
    // 有多少点不在阴影里
    int noShadowCount = 0;
    for( int i = 0; i < NUM_SAMPLES; i ++ ) {
        vec2 sampleCoord = poissonDisk[i] * filterRange + coords.xy;
        vec4 closestDepthVec = texture2D(shadowMap, sampleCoord); 
        float closestDepth = unpack(closestDepthVec);
        float currentDepth = coords.z;
        if(currentDepth < closestDepth + 0.01){
            noShadowCount += 1;
        }
    }

    float shadow = float(noShadowCount) / float(NUM_SAMPLES);
    return shadow;
}

float useShadowMap(sampler2D shadowMap, vec4 shadowCoord){
  float  bias = Bias();
  vec4 depthpack =texture2D(shadowMap,shadowCoord.xy);
  float depthUnpack =unpack(depthpack);
   // 检查当前片段是否在阴影中
  if(depthUnpack > shadowCoord.z -bias)
      return 1.0;
  return 0.0;
}

float findBlocker( sampler2D shadowMap,  vec2 uv, float zReceiver ) {
    poissonDiskSamples(uv);

    float textureSize = 2048.0;
    float filterStride = 8.0;
    float filterRange = 1.0 / textureSize * filterStride;

    int shadowCount = 0;
    float blockDepth = 0.0;
    for( int i = 0; i < NUM_SAMPLES; i ++ ) {
        vec2 sampleCoord = poissonDisk[i] * filterRange + uv;
        vec4 closestDepthVec = texture2D(shadowMap, sampleCoord); 
        float closestDepth = unpack(closestDepthVec);
        if(zReceiver > closestDepth + Bias()){
            blockDepth += closestDepth;
            shadowCount += 1;
        }
    }

    if( shadowCount == NUM_SAMPLES ) return 2.33;
    return blockDepth / float(shadowCount);
}

float PCSS(sampler2D shadowMap, vec4 coords){
    float zReceiver = coords.z;

    // STEP 1: avgblocker depth
    float zBlocker = findBlocker(shadowMap, coords.xy, zReceiver);
    if(zBlocker < EPS) return 1.0;
    if(zBlocker > 1.0) return 0.0;

    // STEP 2: penumbra size
    float wPenumbra = (zReceiver - zBlocker) / zBlocker;

    // STEP 3: filtering
    float textureSize = 2048.0;
    float filterStride = 8.0;
    float filterRange = 1.0 / textureSize * filterStride * wPenumbra;
    int noShadowCount = 0;
    for( int i = 0; i < NUM_SAMPLES; i ++ ) {
        vec2 sampleCoord = poissonDisk[i] * filterRange + coords.xy;
        vec4 closestDepthVec = texture2D(shadowMap, sampleCoord); 
        float closestDepth = unpack(closestDepthVec);
        float currentDepth = coords.z;
        if(currentDepth < closestDepth + Bias()){
            noShadowCount += 1;
        }
    }

    float shadow = float(noShadowCount) / float(NUM_SAMPLES);
    return shadow;
}

void main(void) {
    vec3 ambient = ka * vec3(0.3);

    float visibility;
    vec3 shadowCoord = vPositionFromLight.xyz / vPositionFromLight.w;
    shadowCoord = shadowCoord * 0.5 + 0.5;

    //visibility = PCF(uShadowMap, vec4(shadowCoord, 1.0));
    visibility = PCSS(uShadowMap, vec4(shadowCoord, 1.0));
    
    vec3 color = PBRcolor() * visibility + ambient;
    gl_FragColor = vec4(color, 1.0);
}