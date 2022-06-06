#version 330 core
uniform vec3 uLightDir;
uniform vec3 uCameraPos;
uniform vec3 uLightRadiance;
uniform sampler2D uGDiffuse;
uniform sampler2D uGDepth;
uniform sampler2D uGNormalWorld;
uniform sampler2D uGShadow;
uniform sampler2D uGColor;
// uniform sampler2D uAlbedoMap;
uniform vec3 uColor;

in mat4 vWorldToScreen;
in highp vec4 vPosWorld;
in vec2 vTextureCoord;
in vec3 vNormal;

#define PI 3.1415926535897932384626433832795
#define M_PI 3.1415926535897932384626433832795
#define TWO_PI 6.283185307
#define INV_PI 0.31830988618
#define INV_TWO_PI 0.15915494309

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

vec3 PBRcolor(vec3 wi,vec3 wo,vec2 uv)
{
    vec3 Diffuse = texture2D(uGDiffuse,uv).xyz;
    float uRoughness = Diffuse.x;
    float uMetallic = Diffuse.y;
    // vec3 albedo = pow(texture2D(uAlbedoMap, vTextureCoord).rgb, vec3(2.2));
    // if(albedo==vec3(0.0)) albedo=uColor;
    vec3 albedo=texture2D(uGColor,uv).xyz;
    // vec3 albedo=uColor;
    vec3 N = texture2D(uGNormalWorld,uv).xyz;
    N=normalize((N-vec3(0.5))*2);
    // vec3 N=normalize(vNormal);
    // return N;
    vec3 V = normalize(wo);
    float NdotV = max(dot(N, V), 0.0);
    vec3 F0 = vec3(0.04); 
    F0 = mix(F0, albedo,uMetallic);

    vec3 Lo = vec3(0.0);

    vec3 L = normalize(wi);
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
    // vec3 color = Lo + vec3(0.001) * texture2D(uAlbedoMap, vTextureCoord).rgb;
    vec3 color=Lo;
    // vec3 color = Lo;

    color = color / (color + vec3(1.0));
    return color;
}

float Rand1(inout float p) {
  p = fract(p * .1031);
  p *= p + 33.33;
  p *= p + p;
  return fract(p);
}

vec2 Rand2(inout float p) {
  return vec2(Rand1(p), Rand1(p));
}

float InitRand(vec2 uv) {
	vec3 p3  = fract(vec3(uv.xyx) * .1031);
  p3 += dot(p3, p3.yzx + 33.33);
  return fract((p3.x + p3.y) * p3.z);
}

// uniform sample
vec3 SampleHemisphereUniform(inout float s, out float pdf) {
  vec2 uv = Rand2(s);
  float z = uv.x;
  float phi = uv.y * TWO_PI;
  float sinTheta = sqrt(1.0 - z*z);
  vec3 dir = vec3(sinTheta * cos(phi), sinTheta * sin(phi), z);
  pdf = INV_TWO_PI;
  return dir;
}

// cos importance sample
vec3 SampleHemisphereCos(inout float s, out float pdf) {
  vec2 uv = Rand2(s);
  float z = sqrt(1.0 - uv.x);
  float phi = uv.y * TWO_PI;
  float sinTheta = sqrt(uv.x);
  vec3 dir = vec3(sinTheta * cos(phi), sinTheta * sin(phi), z);
  pdf = z * INV_PI;
  return dir;
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

vec4 Project(vec4 a) {
  return a / a.w;
}

float GetDepth(vec3 posWorld) {
  float depth = (vWorldToScreen * vec4(posWorld, 1.0)).w;
  return depth;
}

/*
 * Transform point from world space to screen space([0, 1] x [0, 1])
 *
 */
vec2 GetScreenCoordinate(vec3 posWorld) {
  vec2 uv = Project(vWorldToScreen * vec4(posWorld, 1.0)).xy * 0.5 + 0.5;
  return uv;
}

float GetGBufferDepth(vec2 uv) {
  float depth = texture2D(uGDepth, uv).x;
  if (depth < 1e-2) {
    depth = 1000.0;
  }
  return depth;
}

vec3 GetGBufferNormalWorld(vec2 uv) {
  vec3 normal = texture2D(uGNormalWorld, uv).xyz;
  return normal;
}

// vec3 GetGBufferPosWorld(vec2 uv) {
//   vec3 posWorld = texture2D(uGPosWorld, uv).xyz;
//   return posWorld;
// }

float GetGBufferuShadow(vec2 uv) {
  float visibility = texture2D(uGShadow, uv).x;
  return visibility;
}


/*
 * Evaluate diffuse bsdf value.
 *
 * wi, wo are all in world space.
 * uv is in screen space, [0, 1] x [0, 1].
 *
 */

/*
 * Evaluate directional light with shadow map
 * uv is in screen space, [0, 1] x [0, 1].
 *
 */
float visibility(vec2 uv) {
  float Visi=GetGBufferuShadow(uv);
  float Le = 0.0;
  if(Visi>0.0) Le=Visi;
  else Le=0.01;
  return Le;
//   return vec3(1.0);
}

#define INIT_STEP 0.8
#define MAX_STEPS 20
#define EPS 1e-3
#define THRES 0.5
bool outScreen(vec3 pos){
  vec2 uv = GetScreenCoordinate(pos);
  return any(bvec4(lessThan(uv, vec2(0.0)), greaterThan(uv, vec2(1.0))));
}
bool atFront(vec3 pos){
  return GetDepth(pos) < GetGBufferDepth(GetScreenCoordinate(pos));
}
bool hasInter(vec3 pos, vec3 dir, out vec3 hitPos){
  float d1 = GetGBufferDepth(GetScreenCoordinate(pos)) - GetDepth(pos) + EPS;
  float d2 = GetDepth(pos + dir) - GetGBufferDepth(GetScreenCoordinate(pos + dir)) + EPS;
  // if(d1 < THRES && d2 < THRES){
    if( d1 < THRES){
    hitPos = pos + dir * d1 / (d1 + d2);
    return true;
  }  
  return false;
}

bool RayMarch(vec3 ori, vec3 dir, out vec3 hitPos) {
  bool intersect = false, firstinter = false;
  float st = INIT_STEP;
  vec3 current = ori;
  for (int i = 0;i < MAX_STEPS;i++){
    if(outScreen(current)){
      break;
    }
    else if(atFront(current + dir * st)){
      current += dir * st;
    }else{
      // hit then move back
      firstinter = true;
      if(st < EPS){
        if(hasInter(current, dir * st * 2.0, hitPos)){
          intersect = true;
        }
        break;
      }
    }
    if(firstinter)
      st *= 0.5;
  }
  return intersect;
}

// cos GGX
vec3 SampleHemisphereGGX(inout float s, out float pdf,vec2 uv,vec3 V){
    vec2 E=Rand2(s);
    float a2=pow(texture2D(uGDiffuse,uv).x,4);
    float Phi = 2 * PI * E.x;
    float CosTheta = sqrt( (1 - E.y) / ( 1 + (a2 - 1) * E.y ) );
    float SinTheta = sqrt( 1 - CosTheta * CosTheta );

    vec3 H;
    H.x = SinTheta * cos( Phi );
    H.y = SinTheta * sin( Phi );
    H.z = CosTheta;

    float d = ( CosTheta * a2 - CosTheta ) * CosTheta + 1;
    float D = a2 / ( PI*d*d );
    vec3 L=normalize(H * 2.0f * dot(V, H) - V);
    pdf = D * CosTheta;

    return L;
}

#define SAMPLE_NUM 20

void main() {
  float s = InitRand(gl_FragCoord.xy);
  vec2 uv=GetScreenCoordinate(vPosWorld.xyz);
  vec3 CameraDir=-normalize(vPosWorld.xyz-uCameraPos);
  
  vec3 L = vec3(0.0);
  L = visibility(uv)*PBRcolor(uLightDir,CameraDir,uv);
  vec3 L_indirect = vec3(0.0);
  // uniform
  for(int i=0;i<SAMPLE_NUM;i++){
    float pdf=0.0;
    vec3 b1,b2;
    // vec3 dir=SampleHemisphereCos(s,pdf);
    // vec3 dir=SampleHemisphereGGX(s,pdf,uv,CameraDir);
    vec3 dir=SampleHemisphereUniform(s,pdf);
    LocalBasis(GetGBufferNormalWorld(uv),b1,b2);
    dir=dir.x*b1+dir.y*b2+dir.z*GetGBufferNormalWorld(uv);
    vec3 hit=vec3(0.0);
    if(RayMarch(vPosWorld.xyz,dir,hit)){
      // vec3 l=PBRcolor(dir,CameraDir,uv)*PBRcolor(uLightDir,dir,GetScreenCoordinate(hit))*visibility(GetScreenCoordinate(hit))/pdf;
      vec3 l=PBRcolor(dir,CameraDir,uv)*PBRcolor(uLightDir,dir,GetScreenCoordinate(hit))*uLightRadiance/pdf;
      L_indirect=L_indirect+l;
      // L_indirect=vec3(20.0);
    }
  }
  L_indirect=L_indirect/float (SAMPLE_NUM);
  L=L+L_indirect;
  vec3 color = pow(clamp(L, vec3(0.0), vec3(1.0)), vec3(1.0 / 2.2));
  // vec3 color=pow(L,vec3(1.0/2.2));
  // vec3 color = L + vec3(0.09);

  gl_FragColor = vec4(vec3(color.rgb), 1.0);
}
