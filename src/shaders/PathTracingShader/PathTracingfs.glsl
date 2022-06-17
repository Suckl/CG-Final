#version 330 core

in vec3 pix;
out vec4 fragColor;

// ----------------------------------------------------------------------------- //

uniform uint frameCounter;
uniform int nTriangles;
uniform int nNodes;
uniform int width;
uniform int height;
uniform int hdrResolution;

uniform samplerBuffer triangles;
uniform samplerBuffer nodes;

uniform sampler2D lastFrame;
uniform sampler2D hdrMap;
uniform sampler2D hdrCache;

uniform vec3 uCameraPos;
uniform mat4 cameraRotate;

// ----------------------------------------------------------------------------- //

#define PI              3.1415926
#define INF             114514.0
#define SIZE_TRIANGLE   9
#define SIZE_BVHNODE    4

// ----------------------------------------------------------------------------- //

struct Triangle {
    vec3 p1, p2, p3;
    vec3 n1, n2, n3;
};

struct BVHNode {
    int left;
    int right;
    int n;
    int index; 
    vec3 AA, BB;
};

struct Material {
    vec3 emissive;
    vec3 baseColor;

    float roughness;
    float metallic;
    float specular;

    float specularTint;
    float subsurface;
    float anisotropic;

    float sheen;
    float sheenTint;
    float clearcoat;
};

struct Ray {
    vec3 startPoint;
    vec3 direction;
};

struct HitResult {
    bool isHit;
    bool isInside; 
    float distance;
    vec3 hitPoint;
    vec3 normal;
    vec3 viewDir;
    Material material;
};

struct Reservoir {
    int M; // number of samples seen so far
    int y; // outputSample
    float W; // weightSum
};

// ----------------------------------------------------------------------------- //

uint seed = uint(
    uint((pix.x * 0.5 + 0.5) * width)  * uint(1973) + 
    uint((pix.y * 0.5 + 0.5) * height) * uint(9277) + 
    uint(frameCounter) * uint(26699)) | uint(1);

uint wang_hash(inout uint seed) {
    seed = uint(seed ^ uint(61)) ^ uint(seed >> uint(16));
    seed *= uint(9);
    seed = seed ^ (seed >> 4);
    seed *= uint(0x27d4eb2d);
    seed = seed ^ (seed >> 15);
    return seed;
}
 
float rand() {
    return float(wang_hash(seed)) / 4294967296.0;
}

// ----------------------------------------------------------------------------- //

const uint V[8*32] = uint[](
    2147483648u,1073741824u,536870912u,268435456u,134217728u,67108864u,33554432u,16777216u,8388608u,4194304u,2097152u,1048576u,524288u,262144u,131072u,65536u,32768u,16384u,8192u,4096u,2048u,1024u,512u,256u,128u,64u,32u,16u,8u,4u,2u,1u,2147483648u,3221225472u,2684354560u,4026531840u,2281701376u,3422552064u,2852126720u,4278190080u,2155872256u,3233808384u,2694840320u,4042260480u,2290614272u,3435921408u,2863267840u,4294901760u,2147516416u,3221274624u,2684395520u,4026593280u,2281736192u,3422604288u,2852170240u,4278255360u,2155905152u,3233857728u,2694881440u,4042322160u,2290649224u,3435973836u,2863311530u,4294967295u,2147483648u,3221225472u,1610612736u,2415919104u,3892314112u,1543503872u,2382364672u,3305111552u,1753219072u,2629828608u,3999268864u,1435500544u,2154299392u,3231449088u,1626210304u,2421489664u,3900735488u,1556135936u,2388680704u,3314585600u,1751705600u,2627492864u,4008611328u,1431684352u,2147543168u,3221249216u,1610649184u,2415969680u,3892340840u,1543543964u,2382425838u,3305133397u,2147483648u,3221225472u,536870912u,1342177280u,4160749568u,1946157056u,2717908992u,2466250752u,3632267264u,624951296u,1507852288u,3872391168u,2013790208u,3020685312u,2181169152u,3271884800u,546275328u,1363623936u,4226424832u,1977167872u,2693105664u,2437829632u,3689389568u,635137280u,1484783744u,3846176960u,2044723232u,3067084880u,2148008184u,3222012020u,537002146u,1342505107u,2147483648u,1073741824u,536870912u,2952790016u,4160749568u,3690987520u,2046820352u,2634022912u,1518338048u,801112064u,2707423232u,4038066176u,3666345984u,1875116032u,2170683392u,1085997056u,579305472u,3016343552u,4217741312u,3719483392u,2013407232u,2617981952u,1510979072u,755882752u,2726789248u,4090085440u,3680870432u,1840435376u,2147625208u,1074478300u,537900666u,2953698205u,2147483648u,1073741824u,1610612736u,805306368u,2818572288u,335544320u,2113929216u,3472883712u,2290089984u,3829399552u,3059744768u,1127219200u,3089629184u,4199809024u,3567124480u,1891565568u,394297344u,3988799488u,920674304u,4193267712u,2950604800u,3977188352u,3250028032u,129093376u,2231568512u,2963678272u,4281226848u,432124720u,803643432u,1633613396u,2672665246u,3170194367u,2147483648u,3221225472u,2684354560u,3489660928u,1476395008u,2483027968u,1040187392u,3808428032u,3196059648u,599785472u,505413632u,4077912064u,1182269440u,1736704000u,2017853440u,2221342720u,3329785856u,2810494976u,3628507136u,1416089600u,2658719744u,864310272u,3863387648u,3076993792u,553150080u,272922560u,4167467040u,1148698640u,1719673080u,2009075780u,2149644390u,3222291575u,2147483648u,1073741824u,2684354560u,1342177280u,2281701376u,1946157056u,436207616u,2566914048u,2625634304u,3208642560u,2720006144u,2098200576u,111673344u,2354315264u,3464626176u,4027383808u,2886631424u,3770826752u,1691164672u,3357462528u,1993345024u,3752330240u,873073152u,2870150400u,1700563072u,87021376u,1097028000u,1222351248u,1560027592u,2977959924u,23268898u,437609937u
);

uint grayCode(uint i) {
	return i ^ (i>>1);
}

float sobol(uint d, uint i) {
    uint result = 0u;
    uint offset = d * 32u;
    for(uint j = 0u; i != 0u; i >>= 1u, j++) 
        if((i & 1u)!=0u)
            result ^= V[j+offset];

    return float(result) * (1.0f/float(0xFFFFFFFFU));
}

vec2 sobolVec2(uint i, uint b) {
    float u = sobol(b * 2u, grayCode(i));
    float v = sobol(b * 2u + 1u, grayCode(i));
    return vec2(u, v);
}

vec3 SampleHemisphere(float u, float v) {
    float z = u;
    float r = max(0, sqrt(1.0 - z * z));
    float phi = 2.0 * PI * v;
    return vec3(r * cos(phi), r * sin(phi), z);
}

vec3 toNormalHemisphere(vec3 v, vec3 N) {
    vec3 helper = vec3(1, 0, 0);
    if(abs(N.x) > 0.999)
        helper = vec3(0, 0, 1);
    vec3 tangent = normalize(cross(N, helper));
    vec3 bitangent = normalize(cross(N, tangent));
    return v.x * tangent + v.y * bitangent + v.z * N;
}

vec2 CranleyPattersonRotation(vec2 p) {
    uint pseed = uint(
        uint((pix.x * 0.5 + 0.5) * width)  * uint(1973) + 
        uint((pix.y * 0.5 + 0.5) * height) * uint(9277) + 
        uint(114514/1919) * uint(26699)) | uint(1);
    
    float u = float(wang_hash(pseed)) / 4294967296.0;
    float v = float(wang_hash(pseed)) / 4294967296.0;

    p.x += u;
    if(p.x>1) p.x -= 1;
    if(p.x<0) p.x += 1;

    p.y += v;
    if(p.y>1) p.y -= 1;
    if(p.y<0) p.y += 1;

    return p;
}

// ----------------------------------------------------------------------------- //

Triangle getTriangle(int i) {
    int offset = i * SIZE_TRIANGLE;
    Triangle t;

    t.p1 = texelFetch(triangles, offset + 0).xyz;
    t.p2 = texelFetch(triangles, offset + 1).xyz;
    t.p3 = texelFetch(triangles, offset + 2).xyz;

    t.n1 = texelFetch(triangles, offset + 3).xyz;
    t.n2 = texelFetch(triangles, offset + 4).xyz;
    t.n3 = texelFetch(triangles, offset + 5).xyz;

    return t;
}

Material getMaterial(int i) {
    Material m;

    int offset = i * SIZE_TRIANGLE;
    vec3 param = texelFetch(triangles, offset + 8).xyz;

    m.emissive = texelFetch(triangles, offset + 6).xyz;
    m.baseColor = texelFetch(triangles, offset + 7).xyz;

    m.roughness = param.x;
    m.metallic = param.y;
    m.specular = param.z;

    return m;
}

BVHNode getBVHNode(int i) {
    BVHNode node;

    int offset = i * SIZE_BVHNODE;
    ivec3 childs = ivec3(texelFetch(nodes, offset + 0).xyz);
    ivec3 leafInfo = ivec3(texelFetch(nodes, offset + 1).xyz);
    node.left = int(childs.x);
    node.right = int(childs.y);
    node.n = int(leafInfo.x);
    node.index = int(leafInfo.y);

    node.AA = texelFetch(nodes, offset + 2).xyz;
    node.BB = texelFetch(nodes, offset + 3).xyz;

    return node;
}

// ----------------------------------------------------------------------------- //

HitResult hitTriangle(Triangle triangle, Ray ray) {
    HitResult res;
    res.distance = INF;
    res.isHit = false;
    res.isInside = false;

    vec3 p1 = triangle.p1;
    vec3 p2 = triangle.p2;
    vec3 p3 = triangle.p3;

    vec3 S = ray.startPoint;
    vec3 d = ray.direction;
    vec3 N = normalize(cross(p2-p1, p3-p1));

    if (dot(N, d) > 0.0f) {
        N = -N;   
        res.isInside = true;
    }

    if (abs(dot(N, d)) < 0.00001f) return res;

    float t = (dot(N, p1) - dot(S, N)) / dot(d, N);
    if (t < 0.0005f) return res; 

    vec3 P = S + d * t;

    vec3 c1 = cross(p2 - p1, P - p1);
    vec3 c2 = cross(p3 - p2, P - p2);
    vec3 c3 = cross(p1 - p3, P - p3);
    bool r1 = (dot(c1, N) > 0 && dot(c2, N) > 0 && dot(c3, N) > 0);
    bool r2 = (dot(c1, N) < 0 && dot(c2, N) < 0 && dot(c3, N) < 0);

    if (r1 || r2) {
        res.isHit = true;
        res.hitPoint = P;
        res.distance = t;
        res.normal = N;
        res.viewDir = d;

        float alpha = (-(P.x-p2.x)*(p3.y-p2.y) + (P.y-p2.y)*(p3.x-p2.x)) / (-(p1.x-p2.x-0.00005)*(p3.y-p2.y+0.00005) + (p1.y-p2.y+0.00005)*(p3.x-p2.x+0.00005));
        float beta  = (-(P.x-p3.x)*(p1.y-p3.y) + (P.y-p3.y)*(p1.x-p3.x)) / (-(p2.x-p3.x-0.00005)*(p1.y-p3.y+0.00005) + (p2.y-p3.y+0.00005)*(p1.x-p3.x+0.00005));
        float gama  = 1.0 - alpha - beta;
        vec3 Nsmooth = alpha * triangle.n1 + beta * triangle.n2 + gama * triangle.n3;
        Nsmooth = normalize(Nsmooth);
        res.normal = (res.isInside) ? (-Nsmooth) : (Nsmooth);
    }

    return res;
}

float hitAABB(Ray r, vec3 AA, vec3 BB) {
    vec3 invdir = 1.0 / r.direction;

    vec3 f = (BB - r.startPoint) * invdir;
    vec3 n = (AA - r.startPoint) * invdir;

    vec3 tmax = max(f, n);
    vec3 tmin = min(f, n);

    float t1 = min(tmax.x, min(tmax.y, tmax.z));
    float t0 = max(tmin.x, max(tmin.y, tmin.z));

    return (t1 >= t0) ? ((t0 > 0.0) ? (t0) : (t1)) : (-1);
}

// ----------------------------------------------------------------------------- //

HitResult hitArray(Ray ray, int l, int r) {
    HitResult res;
    res.isHit = false;
    res.distance = INF;
    for(int i=l; i<=r; i++) {
        Triangle triangle = getTriangle(i);
        HitResult r = hitTriangle(triangle, ray);
        if(r.isHit && r.distance<res.distance) {
            res = r;
            res.material = getMaterial(i);
        }
    }
    return res;
}

HitResult hitBVH(Ray ray) {
    HitResult res;
    res.isHit = false;
    res.distance = INF;

    int stack[256];
    int sp = 0;

    stack[sp++] = 1;
    while(sp > 0) {
        int top = stack[--sp];
        BVHNode node = getBVHNode(top);
        
        if(node.n > 0) {
            int L = node.index;
            int R = node.index + node.n - 1;
            HitResult r = hitArray(ray, L, R);
            if(r.isHit && r.distance < res.distance) res = r;
            continue;
        }
        
        float d1 = INF;
        float d2 = INF;
        if(node.left > 0) {
            BVHNode leftNode = getBVHNode(node.left);
            d1 = hitAABB(ray, leftNode.AA, leftNode.BB);
        }
        if(node.right > 0) {
            BVHNode rightNode = getBVHNode(node.right);
            d2 = hitAABB(ray, rightNode.AA, rightNode.BB);
        }

        if(d1 > 0 && d2 > 0) {
            if(d1 < d2) {
                stack[sp++] = node.right;
                stack[sp++] = node.left;
            } else {
                stack[sp++] = node.left;
                stack[sp++] = node.right;
            }
        } else if(d1 > 0) {
            stack[sp++] = node.left;
        } else if(d2 > 0) {
            stack[sp++] = node.right;
        }
    }

    return res;
}

// ----------------------------------------------------------------------------- //

float DistributionGGX(vec3 N, vec3 H, float roughness) {
    float a2 = pow(roughness, 4.0);
    float NdotH = clamp(dot(N, H), 0.0, 1.0);
    float d = NdotH * NdotH * (a2 - 1.0) + 1.0;
    return a2 / (PI * d * d);
}

float GeometrySchlickGGX(float NdotV, float roughness) {
    NdotV = clamp(NdotV, 0.0, 1.0);
    float k = (roughness + 1.0) * (roughness + 1.0) / 8.0;    
    return NdotV / (NdotV * (1.0 - k) + k);
}

float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness) {
    return GeometrySchlickGGX(dot(L, N), roughness) * GeometrySchlickGGX(dot(V, N), roughness);
}

vec3 fresnelSchlick(vec3 F0, vec3 V, vec3 H) {
    float theta = clamp(dot(V, H), 0.0, 1.0);
    return F0 + (1.0 - F0) * pow(1.0 - theta, 5.0);
}

vec3 PBRcolor(vec3 V, vec3 N, vec3 L, in Material material) {
    // V - view N - normal L - wi
    vec3 albedo = material.baseColor;
    float NdotV = max(dot(N, V), 0.0);
    vec3 F0 = vec3(0.04); 
    F0 = mix(F0, albedo, material.metallic);

    vec3 Lo = vec3(0.0);

    vec3 H = normalize(V + L);
    float NdotL = max(dot(N, L), 0.0); 

    vec3 radiance = vec3(30, 20, 10);

    float NDF = DistributionGGX(N, H, material.roughness);   
    float G   = GeometrySmith(N, V, L, material.roughness);
    vec3 F = fresnelSchlick(F0, V, H);

    vec3 numerator    = NDF * G * F; 
    float denominator = max((4.0 * NdotL * NdotV), 0.001);
    vec3 BRDF = numerator / denominator;

    Lo += BRDF * radiance * NdotL;
    // vec3 color = Lo + vec3(0.001) * texture2D(uAlbedoMap, vTextureCoord).rgb;
    vec3 color = Lo;

    return color;
}

// ----------------------------------------------------------------------------- //

vec2 toSphericalCoord(vec3 v) {
    vec2 uv = vec2(atan(v.z, v.x), asin(v.y));
    uv /= vec2(2.0 * PI, PI);
    uv += 0.5;
    uv.y = 1.0 - uv.y;
    return uv;
}

vec3 hdrColor(vec3 L) {
    vec2 uv = toSphericalCoord(normalize(L));
    vec3 color = texture2D(hdrMap, uv).rgb;
    return color;
}

float hdrPdf(vec3 L, int hdrResolution) {
    vec2 uv = toSphericalCoord(normalize(L));

    float pdf = texture2D(hdrCache, uv).b;
    float theta = PI * (0.5 - uv.y);
    float sin_theta = max(sin(theta), 1e-10);

    float p_convert = float(hdrResolution * hdrResolution / 2) / (2.0 * PI * PI * sin_theta);  
    
    return pdf * p_convert;
}
// ----------------------------------------------------------------------------- //

// Importance Sampling
vec3 SampleHdr(float xi_1, float xi_2) {
    vec2 xy = texture2D(hdrCache, vec2(xi_1, xi_2)).rg;
    xy.y = 1.0 - xy.y; 

    float phi = 2.0 * PI * (xy.x - 0.5);
    float theta = PI * (xy.y - 0.5);

    vec3 L = vec3(cos(theta)*cos(phi), sin(theta), cos(theta)*sin(phi));

    return L;
}

vec3 SampleCosineHemisphere(float r1, float r2, vec3 N) {
    float r = sqrt(r1);
    float theta = r2 * 2.0 * PI;
    float x = r * cos(theta);
    float y = r * sin(theta);
    float z = sqrt(1.0 - x * x - y * y);

    vec3 L = toNormalHemisphere(vec3(x, y, z), N);
    return L;
}

float CosinePDF(vec3 L, vec3 N) {
    return dot(L, N) / PI;
}

vec3 SampleHemisphereGGX(float r1, float r2, out float pdf, vec3 V, float diffuse){
    vec2 E = vec2(r1, r2);
    float a2 = pow(diffuse, 4);
    float Phi = 2 * PI * E.x;
    float CosTheta = sqrt( (1 - E.y) / ( 1 + (a2 - 1) * E.y ) );
    float SinTheta = sqrt( 1 - CosTheta * CosTheta );

    vec3 H;
    H.x = SinTheta * cos( Phi );
    H.y = SinTheta * sin( Phi );
    H.z = CosTheta;

    float d = ( CosTheta * a2 - CosTheta ) * CosTheta + 1;
    float D = a2 / ( PI * d * d );
    vec3 L = normalize(H * 2.0f * dot(V, H) - V);
    pdf = D * CosTheta / (4 * dot (V,H));
    return L;
}

float GGXPDF(vec3 L, vec3 N, float diffuse) {
    return 1.0f;
}

float PowerHeuristic(float a, float b) {
    float t = a * a;
    return t / (b*b + t);
}

vec3 SampleIndirectLight(float r1, float r2, float r3, out float pdf, vec3 V, vec3 N, Material material) {
    float r_diffuse = (1.0 - material.metallic);
    float r_specular = 1.0;
    float r_sum = r_diffuse + r_specular;

    float p_diffuse = r_diffuse / r_sum;
    if(r3 < p_diffuse) {
        vec3 L = SampleCosineHemisphere(r1, r2, N);
        pdf = CosinePDF(L, N);
        return L;
    } else {
        return SampleHemisphereGGX(r1, r2, pdf, V, material.roughness);
    }
}

float IndirectLightPDF(vec3 L, vec3 N, Material material) {
    // float r_diffuse = (1.0 - material.metallic);
    // float r_specular = 1.0;
    // float r_sum = r_diffuse + r_specular;

    // float p_diffuse = r_diffuse / r_sum;
    // float p_specular = r_specular / r_sum;

    // float pdf_diffuse = CosinePDF(L, N);
    // // float pdf_specular = GGXPDF(L, N, material.roughness);
    // float pdf_specular = 0.0;

    // return p_diffuse * pdf_diffuse + p_specular * pdf_specular;
    return CosinePDF(L, N);
}

// ----------------------------------------------------------------------------- //
#define N_SAMPLES 8
#define M_SAMPLES 32

// Restir
Reservoir UpdateReservoir(Reservoir r, int x, float w) {
    r.W += w;
    r.M++;
    if (rand() < (w / r.W)) {
        r.y = x;
    }
    return r;
}

Reservoir ReservoirSampling() {
    Reservoir r;
    for(int i = 0; i < M_SAMPLES; ++i) {
        // r.update(S[i], weight(S[i]))
    }
    return r;
}

Reservoir RIS() {
    Reservoir r;
    for(int i = 0; i < M_SAMPLES; ++i) {
        // generate x_i
        //r.Update
    }
    // r.W
    return r;
}

Reservoir reservoirReus() {
    Reservoir reservoir;
    
    // Generate initial candidate
    
    // Evaluate visibility for initial candidates

    // Temporal reuse

    // Spatial reuse


    return reservoir;
}

// ----------------------------------------------------------------------------- //

// Only 2 Bounce
vec3 pathTracing(HitResult hit, uint maxBounce) {

    vec3 Lo = vec3(0);
    vec3 history = vec3(1);

    for(uint bounce = 0u; bounce < maxBounce; bounce++) {
        vec3 V = -hit.viewDir;
        vec3 N = hit.normal;
        float pdf_light;
        float pdf_indirectLight;

        // direct light
        Ray hdrTestRay;
        hdrTestRay.startPoint = hit.hitPoint;
        hdrTestRay.direction = SampleHdr(rand(), rand());

        if(dot(N, hdrTestRay.direction) > 0.0) {         
            HitResult hdrHit = hitBVH(hdrTestRay);
            
            if(!hdrHit.isHit) {
                vec3 L = hdrTestRay.direction;
                vec3 color = hdrColor(L);
                pdf_light = hdrPdf(L, hdrResolution);
                vec3 f_r = PBRcolor(V, N, L, hit.material);

                pdf_indirectLight = IndirectLightPDF(L, N, hit.material);
                
                float mis_weight = PowerHeuristic(pdf_light, pdf_indirectLight);
                Lo += mis_weight * history * color * f_r * dot(N, L) / pdf_light;
            }
        }

        // indirect light
        vec2 uv = sobolVec2(frameCounter + 1u, bounce);
        uv = CranleyPattersonRotation(uv);
        float r1 = uv.x;
        float r2 = uv.y;
        float r3 = rand();

        vec3 wi = SampleIndirectLight(r1, r2, r3, pdf_indirectLight, V, N, hit.material);
        float cosine_o = max(0, dot(V, N)); 
        float cosine_i = max(0, dot(wi, N)); 
        if(cosine_i <= 0.0) break;

        Ray randomRay;
        randomRay.startPoint = hit.hitPoint;
        randomRay.direction = wi;
        HitResult newHit = hitBVH(randomRay);

        vec3 f_r = PBRcolor(V, N, wi, hit.material);
        if(pdf_indirectLight <= 0.0) break;

        if(!newHit.isHit) {
            vec3 color = hdrColor(wi);
            float pdf_light = hdrPdf(wi, hdrResolution);

            float mis_weight = PowerHeuristic(pdf_light, pdf_indirectLight);
            Lo += mis_weight * history * color *  f_r * cosine_i / pdf_indirectLight;

            break;
        }
        
        vec3 Le = newHit.material.emissive;
        Lo += history * Le * f_r * cosine_i / pdf_indirectLight;
        
        hit = newHit;
        history *= f_r * cosine_i / pdf_indirectLight;
    }
    
    return Lo;
}

// ----------------------------------------------------------------------------- //

void main() {
    Ray ray;
    vec3 color = vec3(0);
    int spp = 1;
    
    ray.startPoint = uCameraPos;
    for(int i = 0; i < spp; ++i) {
        vec2 AA = vec2((rand() - 0.5) / float(width), (rand() - 0.5) / float(height));
        vec4 dir = cameraRotate * vec4(pix.xy + AA, -1.5, 0.0);
        ray.direction = normalize(dir.xyz);

        // primary hit
        HitResult firstHit = hitBVH(ray);  
    
        if(!firstHit.isHit) {
            color += vec3(0, 0, 0);
            color += hdrColor(ray.direction);
        } else {
            vec3 Le = firstHit.material.emissive;
            vec3 Li = pathTracing(firstHit, 2u);
            color += Le + Li;
        } 
    }
    color /= spp;
    
    vec3 lastColor = texture2D(lastFrame, pix.xy * 0.5 + 0.5).rgb;
    color = mix(lastColor, color, 1.0 / float(frameCounter + 1u));
    
    gl_FragData[0] = vec4(color, 1.0);
}
