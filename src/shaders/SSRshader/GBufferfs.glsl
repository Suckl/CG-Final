#version 330 core
uniform float roughness;
uniform float metallic;
// uniform sampler2D uKd;
// uniform sampler2D uNt;
uniform sampler2D uShadowMap;
uniform vec3 uLightPos;
#define NUM_SAMPLES 50
#define NUM_RINGS 10
in mat4 vWorldToLight;
in vec2 vTextureCoord;
in vec4 vPosWorld;
in vec3 vNormalWorld;
in float vDepth;
#define EPS 1e-3
#define PI 3.141592653589793
#define PI2 6.283185307179586

// float SimpleShadowMap(vec3 posWorld,float bias){
//   vec4 posLight = vWorldToLight * vec4(posWorld, 1.0);
//   vec2 shadowCoord = clamp(posLight.xy * 0.5 + 0.5, vec2(0.0), vec2(1.0));
//   float depthSM = texture2D(uShadowMap, shadowCoord).x;
//   float depth = (posLight.z * 0.5 + 0.5) * 100.0;
//   return step(0.0, depthSM - depth + bias);
// }

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

void LocalBasis(vec3 n, out vec3 b1, out vec3 b2) {
  float sign_ = sign(n.z);
  if (n.z == 0.0) {
    sign_ = 1.0;
  }
  float a = -1.0 / (sign_ + n.z);
  float b = n.x * n.y * a;
  b1 = vec3(1.0 + sign_ * n.x * n.x * a, sign_ * b, -sign_ * n.x);
  b2 = vec3(b, sign_ + n.y * n.y * a, -n.y);
}

float unpack(vec4 rgbaDepth) {
    const vec4 bitShift = vec4(1.0, 1.0/256.0, 1.0/(256.0*256.0), 1.0/(256.0*256.0*256.0));
    float depth =dot(rgbaDepth, bitShift) ;

    if(abs(depth)<EPS){
      depth=1.0;
    }

    return  depth;
}

float Bias(){
    vec3 uLightDir = normalize(uLightPos - vPosWorld.xyz);
    float bias = max(0.05 * (1.0 - dot(vNormalWorld, uLightDir)), 0.005);
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
    poissonDiskSamples(coords.xy);
    //uniformDiskSamples(coords.xy);
    float textureSize = 2048.0;
    float filterStride = 5.0;
    float filterRange = 1.0 / textureSize * filterStride;
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

// vec3 ApplyTangentNormalMap() {
//   vec3 t, b;
//   LocalBasis(vNormalWorld, t, b);
//   vec3 nt = texture2D(uNt, vTextureCoord).xyz * 2.0 - 1.0;
//   nt = normalize(nt.x * t + nt.y * b + nt.z * vNormalWorld);
//   return nt;
// }

void main(void) {
  //diffuse 
  gl_FragData[0] = vec4(roughness,metallic,1.0, 1.0);
  //depth
  gl_FragData[1] = vec4(vec3(vDepth), 1.0);
  //normal
  gl_FragData[2] = vec4(vNormalWorld, 1.0);
  //visible
  gl_FragData[3] = vec4(vec3(PCF(uShadowMap,vPosWorld)), 1.0);
  //position
  gl_FragData[4] = vec4(vec3(vPosWorld.xyz), 1.0);
}
