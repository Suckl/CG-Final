
#include"scene.h"
#include"global.h"
#include "baseFunc.h"

const std::string modelPath = "../media/sphere.obj";
const std::string earthTexturePath = "../media/earthmap.jpg";
const std::string planetTexturePath = "../media/planet_Quom1200.png";

const std::vector<std::string> skyboxTexturePaths = {
	"../media/starfield/Right_Tex.jpg",
	"../media/starfield/Left_Tex.jpg",
	"../media/starfield/Up_Tex.jpg",
	"../media/starfield/Down_Tex.jpg",
	"../media/starfield/Front_Tex.jpg",
	"../media/starfield/Back_Tex.jpg"
};

Scene::Scene(const Options& options): Application(options) {

    
	// init scene
    if(!SceneLoad()){
        addModel(modelPath,"earth");
        addTexture(earthTexturePath,"earthTexture");
        addTexture(planetTexturePath,"planetTexture");
    }

	// init camera
	_camera.reset(new PerspectiveCamera(
		glm::radians(50.0f), 1.0f * _windowWidth / _windowHeight, 0.1f, 10000.0f));
    _camera->position.y = 10.0f;
	_camera->position.z = 30.0f;

	// init light
	_directionlight.reset(new DirectionalLight());
    _directionlight->position = glm::vec3(2.33f, 20.0f, 5.0f);

    {
        // const std::string lightCubePath = "../media/plane.obj";
        // _lightlist.ModelList.push_back(nullptr);
        // _lightlist.ModelList[0].reset(new Model(lightCubePath));
        // _lightlist.filepath.push_back(lightCubePath);
        // _lightlist.visible.push_back(true);
        // _lightlist.Color.push_back(glm::vec3(1.0));
        // _lightlist.ModelList[0]->position = glm::vec3(8.0f, 8.0f, 5.0f);
        // _lightlist.ModelList[0]->scale = glm::vec3(2.0f);

        // _lightlist.ModelList.push_back(nullptr);
        // _lightlist.ModelList[1].reset(new Model(lightCubePath));
        // _lightlist.filepath.push_back(lightCubePath);
        // _lightlist.visible.push_back(true);
        // _lightlist.Color.push_back(glm::vec3(1.0));
        // _lightlist.ModelList[1]->position = glm::vec3(0.0f, 8.0f, 5.0f);
        // _lightlist.ModelList[1]->scale = glm::vec3(2.0f);

        // _lightlist.ModelList.push_back(nullptr);
        // _lightlist.ModelList[2].reset(new Model(lightCubePath));
        // _lightlist.filepath.push_back(lightCubePath);
        // _lightlist.visible.push_back(true);
        // _lightlist.Color.push_back(glm::vec3(1.0));
        // _lightlist.ModelList[2]->position = glm::vec3(8.0f, 8.0f, -5.0f);
        // _lightlist.ModelList[2]->scale = glm::vec3(2.0f);

        // _lightlist.ModelList.push_back(nullptr);
        // _lightlist.ModelList[3].reset(new Model(lightCubePath));
        // _lightlist.filepath.push_back(lightCubePath);
        // _lightlist.visible.push_back(true);
        // _lightlist.Color.push_back(glm::vec3(1.0));
        // _lightlist.ModelList[3]->position = glm::vec3(-8.0f, 8.0f, -2-5.0f);
        // _lightlist.ModelList[3]->scale = glm::vec3(2.0f);

        // _lightlist.ModelList.push_back(nullptr);
        // _lightlist.ModelList[4].reset(new Model(lightCubePath));
        // _lightlist.filepath.push_back(lightCubePath);
        // _lightlist.visible.push_back(true);
        // _lightlist.Color.push_back(glm::vec3(1.0));
        // _lightlist.ModelList[4]->position = glm::vec3(-8.0f, 8.0f, 5.0f);
        // _lightlist.ModelList[4]->scale = glm::vec3(2.0f);
    }

	// init skybox
	_skybox.reset(new SkyBox(skyboxTexturePaths));
    // init Series
    _serise.max=0;
    // init fullscreen
    _fullscrennquad.reset(new FullscreenQuad);

	// init shaders
    initShader();

    initPathTracingResources();

	// init imgui
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGui::StyleColorsDark();
	ImGui_ImplGlfw_InitForOpenGL(_window, true);
	ImGui_ImplOpenGL3_Init();

    // init depth map FBO
    _depthfbo.reset(new Framebuffer);
    _depthmap.reset(new DataTexture(GL_DEPTH_COMPONENT, _shadowWidth, _shadowHeight, GL_DEPTH_COMPONENT, GL_FLOAT));
    _depthfbo->bind();
    _depthfbo->attach(*_depthmap,GL_DEPTH_ATTACHMENT);
    _depthfbo->unbind();

    // init GBufferFBO
    _gbufferfbo.reset(new Framebuffer);
    _normaltexture.reset(new DataTexture(GL_RGBA, _windowWidth, _windowHeight, GL_RGBA, GL_FLOAT));
    _visibilitytexture.reset(new DataTexture(GL_RGBA, _windowWidth, _windowHeight, GL_RGBA, GL_FLOAT));
    _colortexture.reset(new DataTexture(GL_RGBA, _windowWidth, _windowHeight, GL_RGBA, GL_FLOAT));
    _diffusetexuture.reset(new DataTexture(GL_RGBA, _windowWidth, _windowHeight, GL_RGBA, GL_FLOAT));
    _depthtexture.reset(new DataTexture(GL_RGBA, _windowWidth, _windowHeight, GL_RGBA, GL_FLOAT));
    _positiontexture.reset(new DataTexture(GL_RGBA16F, _windowWidth, _windowHeight, GL_RGBA, GL_FLOAT));
    _depthgbuffer.reset(new DataTexture(GL_DEPTH_COMPONENT, _windowWidth, _windowHeight, GL_DEPTH_COMPONENT, GL_FLOAT));
    _gbufferfbo->bind();
    const GLenum bufs[6]={GL_COLOR_ATTACHMENT0,GL_COLOR_ATTACHMENT1,GL_COLOR_ATTACHMENT2,GL_COLOR_ATTACHMENT3,GL_COLOR_ATTACHMENT4,
                        GL_COLOR_ATTACHMENT5};
    glDrawBuffers(6,bufs);
    _gbufferfbo->attach(*_diffusetexuture,GL_COLOR_ATTACHMENT0);
    _gbufferfbo->attach(*_depthgbuffer,GL_DEPTH_ATTACHMENT);
    _gbufferfbo->attach(*_depthtexture,GL_COLOR_ATTACHMENT1);
    _gbufferfbo->attach(*_normaltexture,GL_COLOR_ATTACHMENT2);
    _gbufferfbo->attach(*_visibilitytexture,GL_COLOR_ATTACHMENT3);
    _gbufferfbo->attach(*_colortexture,GL_COLOR_ATTACHMENT4);
    _gbufferfbo->attach(*_positiontexture,GL_COLOR_ATTACHMENT5);
    _gbufferfbo->unbind();
    // init filterFBO
    _filterfbo.reset(new Framebuffer);
    _beauty.reset(new DataTexture(GL_RGBA, _windowWidth, _windowHeight, GL_RGBA, GL_FLOAT));
    _depthfilter.reset(new DataTexture(GL_DEPTH_COMPONENT, _windowWidth, _windowHeight, GL_DEPTH_COMPONENT, GL_FLOAT));
    _filterfbo->bind();
    glDrawBuffer(GL_COLOR_ATTACHMENT0);
    _filterfbo->attach(*_beauty,GL_COLOR_ATTACHMENT0);
    _filterfbo->attach(*_depthfilter,GL_DEPTH_ATTACHMENT);
    _filterfbo->unbind();
}

Scene::~Scene() {
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();
}

void Scene::initShader(){
    _pbrShader.reset(new GLSLProgram); 
	_pbrShader->attachVertexShaderFromFile("../../src/shaders/PBRvs.glsl");
	_pbrShader->attachFragmentShaderFromFile("../../src/shaders/PBRfs.glsl");
	_pbrShader->link();

    _shadowShader.reset(new GLSLProgram);
    _shadowShader->attachVertexShaderFromFile("../../src/shaders/ShadowShader/Shadowvs.glsl");
    _shadowShader->attachFragmentShaderFromFile("../../src/shaders/ShadowShader/Shadowfs.glsl");
    _shadowShader->link();

    _shadowMappingShader.reset(new GLSLProgram);
    _shadowMappingShader->attachVertexShaderFromFile("../../src/shaders/ShadowShader/ShadowMappingvs.glsl");
    _shadowMappingShader->attachFragmentShaderFromFile("../../src/shaders/ShadowShader/ShadowMappingfs.glsl");
    _shadowMappingShader->link();

    _pcfShader.reset(new GLSLProgram);
    _pcfShader->attachVertexShaderFromFile("../../src/shaders/ShadowShader/ShadowMappingvs.glsl");
    _pcfShader->attachFragmentShaderFromFile("../../src/shaders/ShadowShader/PCFfs.glsl");
    _pcfShader->link();

    _pcssShader.reset(new GLSLProgram);
    _pcssShader->attachVertexShaderFromFile("../../src/shaders/ShadowShader/ShadowMappingvs.glsl");
    _pcssShader->attachFragmentShaderFromFile("../../src/shaders/ShadowShader/PCSSfs.glsl");
    _pcssShader->link();

    _pointShadowShader.reset(new GLSLProgram);
    _pointShadowShader->attachVertexShaderFromFile("../../src/shaders/ShadowShader/PointShadowsvs.glsl");
    _pointShadowShader->attachGeometryShaderFromFile("../../src/shaders/ShadowShader/PointShadowsgs.glsl");
    _pointShadowShader->attachFragmentShaderFromFile("../../src/shaders/ShadowShader/PointShadowsfs.glsl");
    _pointShadowShader->link();

    _omnidirectionalShader.reset(new GLSLProgram);
    _omnidirectionalShader->attachVertexShaderFromFile("../../src/shaders/ShadowShader/OmnidirectionalShadowvs.glsl");
    _omnidirectionalShader->attachFragmentShaderFromFile("../../src/shaders/ShadowShader/OmnidirectionalShadowfs.glsl");
    _omnidirectionalShader->link();

    _lightCubeShader.reset(new GLSLProgram);
    _lightCubeShader->attachVertexShaderFromFile("../../src/shaders/ShadowShader/LightCubevs.glsl");
    _lightCubeShader->attachFragmentShaderFromFile("../../src/shaders/ShadowShader/LightCubefs.glsl");
    _lightCubeShader->link();

    _gbufferShader.reset(new GLSLProgram);
    _gbufferShader->attachVertexShaderFromFile("../../src/shaders/SSRshader/GBuffervs.glsl");
    _gbufferShader->attachFragmentShaderFromFile("../../src/shaders/SSRshader/GBufferfs.glsl");
    _gbufferShader->link();

    _ssrShader.reset(new GLSLProgram);
    _ssrShader->attachVertexShaderFromFile("../../src/shaders/SSRshader/SSRvs.glsl");
    _ssrShader->attachFragmentShaderFromFile("../../src/shaders/SSRshader/SSRfs.glsl");
    _ssrShader->link();

    _filterShader.reset(new GLSLProgram);
    _filterShader->attachVertexShaderFromFile("../../src/shaders/SSRshader/Filtervs.glsl");
    _filterShader->attachFragmentShaderFromFile("../../src/shaders/SSRshader/Filterfs.glsl");
    _filterShader->link();

}

void Scene::initPathTracingModel(int index, Material m) {
    auto vertices = _objectlist.ModelList[index]->_vertices;
    auto indices = _objectlist.ModelList[index]->_indices;

    mat4 model = _objectlist.ModelList[index]->getModelMatrix();
    mat4 view = _camera->getViewMatrix();
    mat4 projection = _camera->getProjectionMatrix();
    for(auto& v : vertices) {
        v.position = projection * view * model * glm::vec4(v.position, 1.0f);
    }

    // 构建 Triangle 对象数组
    int offset = triangles.size();  // 增量更新
    triangles.resize(offset + indices.size() / 3);
    for (int i = 0; i < indices.size(); i += 3) {
        Triangle& t = triangles[offset + i / 3];
        // 传顶点属性
        t.p1 = vertices[indices[i]].position;
        t.p2 = vertices[indices[i + 1]].position;
        t.p3 = vertices[indices[i + 2]].position;

        t.n1 = vertices[indices[i]].normal;
        t.n2 = vertices[indices[i + 1]].normal;
        t.n3 = vertices[indices[i + 2]].normal;

        // 传材质
        t.material = m;
    }
}

void Scene::initPathTracingLight(int index, Material m) {
    auto vertices = _lightlist.ModelList[index]->_vertices;
    auto indices = _lightlist.ModelList[index]->_indices;

    mat4 model = _lightlist.ModelList[index]->getModelMatrix();
    mat4 view = _camera->getViewMatrix();
    mat4 projection = _camera->getProjectionMatrix();
    for(auto& v : vertices) {
        v.position = projection * view * model * glm::vec4(v.position, 1.0f);
    }

    // 构建 Triangle 对象数组
    int offset = triangles.size();  // 增量更新
    triangles.resize(offset + indices.size() / 3);
    for (int i = 0; i < indices.size(); i += 3) {
        Triangle& t = triangles[offset + i / 3];
        // 传顶点属性
        t.p1 = vertices[indices[i]].position;
        t.p2 = vertices[indices[i + 1]].position;
        t.p3 = vertices[indices[i + 2]].position;

        t.n1 = vertices[indices[i]].normal;
        t.n2 = vertices[indices[i + 1]].normal;
        t.n3 = vertices[indices[i + 2]].normal;

        // 传材质
        t.material = m;
    }
}

int buildBVHwithSAH(std::vector<Triangle>& triangles, std::vector<BVHNode>& nodes, int l, int r, int n) {
    if (l > r) return 0;

    const float INF = 1145141919.0f;
    nodes.push_back(BVHNode());
    int id = nodes.size() - 1;   
    nodes[id].left = nodes[id].right = nodes[id].n = nodes[id].index = 0;
    nodes[id].AA = vec3(1145141919, 1145141919, 1145141919);
    nodes[id].BB = vec3(-1145141919, -1145141919, -1145141919);

    // 计算 AABB
    for (int i = l; i <= r; i++) {
        // 最小点 AA
        float minx = min(triangles[i].p1.x, min(triangles[i].p2.x, triangles[i].p3.x));
        float miny = min(triangles[i].p1.y, min(triangles[i].p2.y, triangles[i].p3.y));
        float minz = min(triangles[i].p1.z, min(triangles[i].p2.z, triangles[i].p3.z));
        nodes[id].AA.x = min(nodes[id].AA.x, minx);
        nodes[id].AA.y = min(nodes[id].AA.y, miny);
        nodes[id].AA.z = min(nodes[id].AA.z, minz);
        // 最大点 BB
        float maxx = max(triangles[i].p1.x, max(triangles[i].p2.x, triangles[i].p3.x));
        float maxy = max(triangles[i].p1.y, max(triangles[i].p2.y, triangles[i].p3.y));
        float maxz = max(triangles[i].p1.z, max(triangles[i].p2.z, triangles[i].p3.z));
        nodes[id].BB.x = max(nodes[id].BB.x, maxx);
        nodes[id].BB.y = max(nodes[id].BB.y, maxy);
        nodes[id].BB.z = max(nodes[id].BB.z, maxz);
    }

    // 不多于 n 个三角形 返回叶子节点
    if ((r - l + 1) <= n) {
        nodes[id].n = r - l + 1;
        nodes[id].index = l;
        return id;
    }

    // 否则递归建树
    float Cost = INF;
    int Axis = 0;
    int Split = (l + r) / 2;
    for (int axis = 0; axis < 3; axis++) {
        // 分别按 x，y，z 轴排序
        if (axis == 0) std::sort(&triangles[0] + l, &triangles[0] + r + 1, cmpx);
        if (axis == 1) std::sort(&triangles[0] + l, &triangles[0] + r + 1, cmpy);
        if (axis == 2) std::sort(&triangles[0] + l, &triangles[0] + r + 1, cmpz);

        // leftMax[i]: [l, i] 中最大的 xyz 值
        // leftMin[i]: [l, i] 中最小的 xyz 值
        std::vector<vec3> leftMax(r - l + 1, vec3(-INF, -INF, -INF));
        std::vector<vec3> leftMin(r - l + 1, vec3(INF, INF, INF));
        // 计算前缀 注意 i-l 以对齐到下标 0
        for (int i = l; i <= r; i++) {
            Triangle& t = triangles[i];
            int bias = (i == l) ? 0 : 1;  // 第一个元素特殊处理

            leftMax[i - l].x = max(leftMax[i - l - bias].x, max(t.p1.x, max(t.p2.x, t.p3.x)));
            leftMax[i - l].y = max(leftMax[i - l - bias].y, max(t.p1.y, max(t.p2.y, t.p3.y)));
            leftMax[i - l].z = max(leftMax[i - l - bias].z, max(t.p1.z, max(t.p2.z, t.p3.z)));

            leftMin[i - l].x = min(leftMin[i - l - bias].x, min(t.p1.x, min(t.p2.x, t.p3.x)));
            leftMin[i - l].y = min(leftMin[i - l - bias].y, min(t.p1.y, min(t.p2.y, t.p3.y)));
            leftMin[i - l].z = min(leftMin[i - l - bias].z, min(t.p1.z, min(t.p2.z, t.p3.z)));
        }

        // rightMax[i]: [i, r] 中最大的 xyz 值
        // rightMin[i]: [i, r] 中最小的 xyz 值
        std::vector<vec3> rightMax(r - l + 1, vec3(-INF, -INF, -INF));
        std::vector<vec3> rightMin(r - l + 1, vec3(INF, INF, INF));
        // 计算后缀 注意 i-l 以对齐到下标 0
        for (int i = r; i >= l; i--) {
            Triangle& t = triangles[i];
            int bias = (i == r) ? 0 : 1;  // 第一个元素特殊处理

            rightMax[i - l].x = max(rightMax[i - l + bias].x, max(t.p1.x, max(t.p2.x, t.p3.x)));
            rightMax[i - l].y = max(rightMax[i - l + bias].y, max(t.p1.y, max(t.p2.y, t.p3.y)));
            rightMax[i - l].z = max(rightMax[i - l + bias].z, max(t.p1.z, max(t.p2.z, t.p3.z)));

            rightMin[i - l].x = min(rightMin[i - l + bias].x, min(t.p1.x, min(t.p2.x, t.p3.x)));
            rightMin[i - l].y = min(rightMin[i - l + bias].y, min(t.p1.y, min(t.p2.y, t.p3.y)));
            rightMin[i - l].z = min(rightMin[i - l + bias].z, min(t.p1.z, min(t.p2.z, t.p3.z)));
        }

        // 遍历寻找分割
        float cost = INF;
        int split = l;
        for (int i = l; i <= r - 1; i++) {
            float lenx, leny, lenz;
            // 左侧 [l, i]
            vec3 leftAA = leftMin[i - l];
            vec3 leftBB = leftMax[i - l];
            lenx = leftBB.x - leftAA.x;
            leny = leftBB.y - leftAA.y;
            lenz = leftBB.z - leftAA.z;
            float leftS = 2.0 * ((lenx * leny) + (lenx * lenz) + (leny * lenz));
            float leftCost = leftS * (i - l + 1);

            // 右侧 [i+1, r]
            vec3 rightAA = rightMin[i + 1 - l];
            vec3 rightBB = rightMax[i + 1 - l];
            lenx = rightBB.x - rightAA.x;
            leny = rightBB.y - rightAA.y;
            lenz = rightBB.z - rightAA.z;
            float rightS = 2.0 * ((lenx * leny) + (lenx * lenz) + (leny * lenz));
            float rightCost = rightS * (r - i);

            // 记录每个分割的最小答案
            float totalCost = leftCost + rightCost;
            if (totalCost < cost) {
                cost = totalCost;
                split = i;
            }
        }
        // 记录每个轴的最佳答案
        if (cost < Cost) {
            Cost = cost;
            Axis = axis;
            Split = split;
        }
    }

    // 按最佳轴分割
    if (Axis == 0) std::sort(&triangles[0] + l, &triangles[0] + r + 1, cmpx);
    if (Axis == 1) std::sort(&triangles[0] + l, &triangles[0] + r + 1, cmpy);
    if (Axis == 2) std::sort(&triangles[0] + l, &triangles[0] + r + 1, cmpz);

    // 递归
    int left  = buildBVHwithSAH(triangles, nodes, l, Split, n);
    int right = buildBVHwithSAH(triangles, nodes, Split + 1, r, n);

    nodes[id].left = left;
    nodes[id].right = right;

    return id;
}

float* calculateHdrCache(float* HDR, int width, int height) {

    float lumSum = 0.0;

    // 初始化 h 行 w 列的概率密度 pdf 并 统计总亮度
    std::vector<std::vector<float>> pdf(height);
    for (auto& line : pdf) line.resize(width);
    for (int i = 0; i < height; i++) {
        for (int j = 0; j < width; j++) {
            float R = HDR[3 * (i * width + j)];
            float G = HDR[3 * (i * width + j) + 1];
            float B = HDR[3 * (i * width + j) + 2];
            float lum = 0.2 * R + 0.7 * G + 0.1 * B;
            pdf[i][j] = lum;
            lumSum += lum;
        }
    }

    // 概率密度归一化
    for (int i = 0; i < height; i++)
        for (int j = 0; j < width; j++)
            pdf[i][j] /= lumSum;
    
    // 累加每一列得到 x 的边缘概率密度
    std::vector<float> pdf_x_margin;
    pdf_x_margin.resize(width);
    for (int j = 0; j < width; j++) 
        for (int i = 0; i < height; i++) 
            pdf_x_margin[j] += pdf[i][j];

    // 计算 x 的边缘分布函数
    std::vector<float> cdf_x_margin = pdf_x_margin;
    for (int i = 1; i < width; i++)
        cdf_x_margin[i] += cdf_x_margin[i - 1];

    // 计算 y 在 X=x 下的条件概率密度函数
    std::vector<std::vector<float>> pdf_y_condiciton = pdf;
    for (int j = 0; j < width; j++) 
        for (int i = 0; i < height; i++) 
            pdf_y_condiciton[i][j] /= pdf_x_margin[j];

    // 计算 y 在 X=x 下的条件概率分布函数
    std::vector<std::vector<float>> cdf_y_condiciton = pdf_y_condiciton;
    for (int j = 0; j < width; j++)
        for (int i = 1; i < height; i++)
            cdf_y_condiciton[i][j] += cdf_y_condiciton[i - 1][j];

    // cdf_y_condiciton 转置为按列存储
    // cdf_y_condiciton[i] 表示 y 在 X=i 下的条件概率分布函数
    std::vector<std::vector<float>> temp = cdf_y_condiciton;
    cdf_y_condiciton = std::vector<std::vector<float>>(width);
    for (auto& line : cdf_y_condiciton) line.resize(height);
    for (int j = 0; j < width; j++)
        for (int i = 0; i < height; i++)
            cdf_y_condiciton[j][i] = temp[i][j];
    
    // 穷举 xi_1, xi_2 预计算样本 xy
    // sample_x[i][j] 表示 xi_1=i/height, xi_2=j/width 时 (x,y) 中的 x
    // sample_y[i][j] 表示 xi_1=i/height, xi_2=j/width 时 (x,y) 中的 y
    // sample_p[i][j] 表示取 (i, j) 点时的概率密度
    std::vector<std::vector<float>> sample_x(height);
    for (auto& line : sample_x) line.resize(width);
    std::vector<std::vector<float>> sample_y(height);
    for (auto& line : sample_y) line.resize(width);
    std::vector<std::vector<float>> sample_p(height);
    for (auto& line : sample_p) line.resize(width);
    for (int j = 0; j < width; j++) {
        for (int i = 0; i < height; i++) {
            float xi_1 = float(i) / height;
            float xi_2 = float(j) / width;
            
            // 用 xi_1 在 cdf_x_margin 中 lower bound 得到样本 x
            int x = std::lower_bound(cdf_x_margin.begin(), cdf_x_margin.end(), xi_1) - cdf_x_margin.begin();
            // 用 xi_2 在 X=x 的情况下得到样本 y
            int y = std::lower_bound(cdf_y_condiciton[x].begin(), cdf_y_condiciton[x].end(), xi_2) - cdf_y_condiciton[x].begin();

            // 存储纹理坐标 xy 和 xy 位置对应的概率密度
            sample_x[i][j] = float(x) / width;
            sample_y[i][j] = float(y) / height;
            sample_p[i][j] = pdf[i][j];
        }
    }
        
    // 整合结果到纹理
    // R,G 通道存储样本 (x,y) 而 B 通道存储 pdf(i, j)
    float* cache = new float[width * height * 3];
    //for (int i = 0; i < width * height * 3; i++) cache[i] = 0.0;

    for (int i = 0; i < height; i++) {
        for (int j = 0; j < width; j++) {
            cache[3 * (i * width + j)] = sample_x[i][j];        // R
            cache[3 * (i * width + j) + 1] = sample_y[i][j];    // G
            cache[3 * (i * width + j) + 2] = sample_p[i][j];    // B
        }
    }   

    return cache;
}

void Scene::initPathTracingResources() {
    Material m;
    m.baseColor = vec3(1, 1, 1);
    for(int i = 0; i < _lightlist.ModelList.size(); ++i){
        m.emissive = vec3(30, 20, 10);
        initPathTracingLight(i, m);
    }

    m.emissive = vec3(0, 0, 0);
    for(int i = 0; i < _objectlist.ModelList.size(); ++i){
        m.roughness = _objectlist.roughness[i];
        // m.metallic = _objectlist.metallic[i];
        m.metallic = 0.8f;
        m.baseColor = _objectlist.Color[i];
        initPathTracingModel(i, m);
    }

    int nTriangles = triangles.size();
    
    std::cout << "Loading Model Finish: " << nTriangles << " triangles" << std::endl;

    bvhNode.left = 255;
    bvhNode.right = 128;
    bvhNode.n = 30;
    bvhNode.AA = vec3(1, 1, 0);
    bvhNode.BB = vec3(0, 1, 0);
    std::vector<BVHNode> nodes{ bvhNode };
    // buildBVH(triangles, nodes, 0, triangles.size() - 1, 8);
    buildBVHwithSAH(triangles, nodes, 0, triangles.size() - 1, 8);
    int nNodes = nodes.size();
    std::cout << "BVH Construction Finish: " << nNodes << " nodes" << std::endl;

    triangles_encoded.resize(nTriangles);
    for (int i = 0; i < nTriangles; i++) {
        Triangle& t = triangles[i];
        Material& m = t.material;
        // 顶点位置
        triangles_encoded[i].p1 = t.p1;
        triangles_encoded[i].p2 = t.p2;
        triangles_encoded[i].p3 = t.p3;
        // 顶点法线
        triangles_encoded[i].n1 = t.n1;
        triangles_encoded[i].n2 = t.n2;
        triangles_encoded[i].n3 = t.n3;
        // 材质
        triangles_encoded[i].emissive = m.emissive;
        triangles_encoded[i].baseColor = m.baseColor;

        triangles_encoded[i].material = vec3(m.roughness, m.metallic, m.specular);
    }

    nodes_encoded.resize(nNodes);
    for (int i = 0; i < nNodes; i++) {
        nodes_encoded[i].childs = vec3(nodes[i].left, nodes[i].right, 0);
        nodes_encoded[i].leafInfo = vec3(nodes[i].n, nodes[i].index, 0);
        nodes_encoded[i].AA = nodes[i].AA;
        nodes_encoded[i].BB = nodes[i].BB;
    }
    std::cout << "Encode Finish" << std::endl;

    // 三角形数组
    GLuint tbo0;
    glGenBuffers(1, &tbo0);
    glBindBuffer(GL_TEXTURE_BUFFER, tbo0);
    glBufferData(GL_TEXTURE_BUFFER, triangles_encoded.size() * sizeof(Triangle_encoded), &triangles_encoded[0], GL_STATIC_DRAW);
    glGenTextures(1, &_trianglesTextureBuffer);
    glBindTexture(GL_TEXTURE_BUFFER, _trianglesTextureBuffer);
    glTexBuffer(GL_TEXTURE_BUFFER, GL_RGB32F, tbo0);

    // BVHNode 数组
    GLuint tbo1;
    glGenBuffers(1, &tbo1);
    glBindBuffer(GL_TEXTURE_BUFFER, tbo1);
    glBufferData(GL_TEXTURE_BUFFER, nodes_encoded.size() * sizeof(BVHNode_encoded), &nodes_encoded[0], GL_STATIC_DRAW);
    glGenTextures(1, &_nodesTextureBuffer);
    glBindTexture(GL_TEXTURE_BUFFER, _nodesTextureBuffer);
    glTexBuffer(GL_TEXTURE_BUFFER, GL_RGB32F, tbo1);

    HDRLoaderResult hdrRes;
    // bool r = HDRLoader::load("./media/chinese_garden_2k.hdr", hdrRes);
    bool r = HDRLoader::load("C:/Users/Hiyori/Documents/GitHub/CG-Final/media/chinese_garden_2k.hdr", hdrRes);

    if(r)
        std::cout << "HDR Load Finish" << std::endl;
    else
        std::cout << "HDR Load Failed" << std::endl;

    _hdrMap = getTextureRGB32F(hdrRes.width, hdrRes.height);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, hdrRes.width, hdrRes.height, 0, GL_RGB, GL_FLOAT, hdrRes.cols);

    float* cache = calculateHdrCache(hdrRes.cols, hdrRes.width, hdrRes.height);
    hdrCache = getTextureRGB32F(hdrRes.width, hdrRes.height);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, hdrRes.width, hdrRes.height, 0, GL_RGB, GL_FLOAT, cache);
    hdrResolution = hdrRes.width;

    //------------------------------------------------------------------------------------------//

    pass1.program = getShaderProgram("../../src/shaders/PathTracingShader/PathTracingfs.glsl", 
        "../../src/shaders/PathTracingShader/PathTracingvs.glsl");
    pass1.colorAttachments.push_back(getTextureRGB32F(pass1.width, pass1.height));
    pass1.colorAttachments.push_back(getTextureRGB32F(pass1.width, pass1.height));
    pass1.colorAttachments.push_back(getTextureRGB32F(pass1.width, pass1.height));
    pass1.bindData();
   
    glUseProgram(pass1.program);
    glUniform1i(glGetUniformLocation(pass1.program, "nTriangles"), triangles.size());
    glUniform1i(glGetUniformLocation(pass1.program, "nNodes"), nodes.size());
    glUniform1i(glGetUniformLocation(pass1.program, "width"), pass1.width);
    glUniform1i(glGetUniformLocation(pass1.program, "height"), pass1.height);
    glUseProgram(0);

    pass2.program = getShaderProgram("../../src/shaders/PathTracingShader/Pass2fs.glsl", 
        "../../src/shaders/PathTracingShader/PathTracingvs.glsl");
    _lastFrame = getTextureRGB32F(pass2.width, pass2.height);
    pass2.colorAttachments.push_back(_lastFrame);
    pass2.bindData();

    pass3.program = getShaderProgram("../../src/shaders/PathTracingShader/Pass3fs.glsl", 
        "../../src/shaders/PathTracingShader/PathTracingvs.glsl");
    pass3.bindData(true);

}

void Scene::handleInput() {
    static bool test=false;
	const float angluarVelocity = 0.1f;
	const float angle = angluarVelocity * static_cast<float>(_deltaTime);
	const glm::vec3 axis = glm::vec3(0.0f, 1.0f, 0.0f);
    _objectlist.ModelList[0]->rotation = glm::angleAxis(angle, axis) * _objectlist.ModelList[0]->rotation;
    if (_keyboardInput.keyStates[GLFW_KEY_ESCAPE] != GLFW_RELEASE) {
		glfwSetWindowShouldClose(_window, true);
		return;
	}

	if (_keyboardInput.keyStates[GLFW_KEY_W] != GLFW_RELEASE&&mouse_capture_flag) {
        frameCounter = 0;
		if(_CameraMode==CameraMode::Free)_camera->position += _camera->getFront() * _cameraMoveSpeed * (float)_deltaTime*1.5;
        else _camera->position += 
            glm::normalize(glm::vec3(_camera->getFront().x,0.0f,_camera->getFront().z)) * _cameraMoveSpeed * (float)_deltaTime;
	}

	if (_keyboardInput.keyStates[GLFW_KEY_A] != GLFW_RELEASE&&mouse_capture_flag) {
        frameCounter = 0;
		if(_CameraMode==CameraMode::Free)_camera->position -= _camera->getRight() * _cameraMoveSpeed * (float)_deltaTime*1.5;
        else _camera->position -= 
            glm::normalize(glm::vec3(_camera->getRight().x,0.0f,_camera->getRight().z)) * _cameraMoveSpeed * (float)_deltaTime;
	}

	if (_keyboardInput.keyStates[GLFW_KEY_S] != GLFW_RELEASE&&mouse_capture_flag) {
        frameCounter = 0;
		if(_CameraMode==CameraMode::Free)_camera->position -= _camera->getFront() * _cameraMoveSpeed * (float)_deltaTime*1.5;
        else _camera->position -= 
            glm::normalize(glm::vec3(_camera->getFront().x,0.0f,_camera->getFront().z)) * _cameraMoveSpeed * (float)_deltaTime;
	}

	if (_keyboardInput.keyStates[GLFW_KEY_D] != GLFW_RELEASE&&mouse_capture_flag) {
        frameCounter = 0;
		if(_CameraMode==CameraMode::Free)_camera->position += _camera->getRight() * _cameraMoveSpeed * (float)_deltaTime*1.5;
        else _camera->position += 
            glm::normalize(glm::vec3(_camera->getRight().x,0.0f,_camera->getRight().z)) * _cameraMoveSpeed * (float)_deltaTime;
	}

    if (_keyboardInput.keyStates[GLFW_KEY_SPACE] != GLFW_RELEASE&&mouse_capture_flag) {
        frameCounter = 0;
		if(_CameraMode==CameraMode::Free)_camera->position += glm::vec3(0.0,1.0,0.0) * _cameraMoveSpeed * (float)_deltaTime*1.5;
	}

    if (_keyboardInput.keyStates[GLFW_KEY_LEFT_ALT] != GLFW_RELEASE&&test==false) {
        test=true;
		mouse_capture_flag=!mouse_capture_flag;
        if(mouse_capture_flag) glfwSetInputMode(_window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        else glfwSetInputMode(_window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
	}
    if(_keyboardInput.keyStates[GLFW_KEY_LEFT_ALT] == GLFW_RELEASE&&test==true){
        test=false;
    }

    if (_mouseInput.move.xCurrent != _mouseInput.move.xOld&&mouse_capture_flag) {
        frameCounter = 0;
		// std::cout << "mouse move in x direction" << std::endl;
		// rotate around world up: glm::vec3(0.0f, 1.0f, 0.0f)
        if(test) _mouseInput.move.xOld=_mouseInput.move.xCurrent;
		const float deltaX = static_cast<float>(_mouseInput.move.xCurrent - _mouseInput.move.xOld);
		const float angle = -cameraRotateSpeed * _deltaTime * deltaX;
		const glm::vec3 axis = { 0.0f, 1.0f, 0.0f };
		/* write your code here */
		// you should know how quaternion works to represent rotation
		_camera->rotation = glm::normalize(glm::quat{cos(angle/2),0.0f,axis.y*sin(angle / 2),0.0f }*_camera->rotation);
		_mouseInput.move.xOld = _mouseInput.move.xCurrent;
	}
	
	if (_mouseInput.move.yCurrent != _mouseInput.move.yOld&&mouse_capture_flag) {
        frameCounter = 0;
		// std::cout << "mouse move in y direction" << std::endl;
		/* write your code here */
		// rotate around local right
        if(test) _mouseInput.move.yOld=_mouseInput.move.yCurrent;
		const float deltaY = static_cast<float>(_mouseInput.move.yCurrent - _mouseInput.move.yOld);
		float angle = -cameraRotateSpeed * _deltaTime * deltaY;
		const glm::vec3 axis = _camera->getRight();
		// you should know how quaternion works to represent rotation
		_camera->rotation = glm::normalize(glm::quat{ cos(angle / 2),axis.x*sin(angle / 2),axis.y*sin(angle / 2),axis.z*sin(angle / 2) }*_camera->rotation);
		_mouseInput.move.yOld = _mouseInput.move.yCurrent;
	}


}

void Scene::renderFrame() {
	showFpsInWindowTitle();
	glClearColor(_clearColor.r, _clearColor.g, _clearColor.b, _clearColor.a);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glEnable(GL_DEPTH_TEST);
	const glm::mat4 projection = _camera->getProjectionMatrix();
	const glm::mat4 view = _camera->getViewMatrix();

 
	// draw scene
	drawList();

    // draw skybox
	_skybox->draw(projection, view);
    
    drawGUI();
}

bool Scene::addModel(const std::string filename,const std::string name){
    _objectlist.ModelList.push_back(nullptr);
    _objectlist.ModelList[_objectlist.ModelList.size()-1].reset(new Model(filename));
    if(!_objectlist.ModelList[_objectlist.ModelList.size()-1]->success){
        _objectlist.ModelList.pop_back();
        return false;
    }
    _objectlist.filepath.push_back(filename);
    _objectlist.objectname.push_back(name);
    _objectlist.visible.push_back(true);
    _objectlist.Color.push_back(glm::vec3(1.0));
    _objectlist.TextureIndex.push_back(0);
    _objectlist.color_flag.push_back(true);
    _objectlist.roughness.push_back(0.5f);
    _objectlist.metallic.push_back(0.5f);
    return true;
}

void Scene::deleteModel(int index){
    _objectlist.filepath.erase(_objectlist.filepath.begin()+index);
    _objectlist.ModelList.erase(_objectlist.ModelList.begin()+index);
    _objectlist.objectname.erase(_objectlist.objectname.begin()+index);
    _objectlist.visible.erase(_objectlist.visible.begin()+index);
    _objectlist.color_flag.erase(_objectlist.color_flag.begin()+index);
    _objectlist.Color.erase(_objectlist.Color.begin()+index);
    _objectlist.roughness.erase(_objectlist.roughness.begin()+index);
    _objectlist.metallic.erase(_objectlist.metallic.begin()+index);
    _objectlist.TextureIndex.erase(_objectlist.TextureIndex.begin()+index);
}

bool Scene::addTexture(const std::string filename,const std::string name){
    _texturelist.texture.push_back(nullptr);
	_texturelist.texture[_texturelist.texture.size()-1].reset(new Texture2D(filename));
    if(!_texturelist.texture[_texturelist.texture.size()-1]->success){
        _texturelist.texture.pop_back();
        return false;
    }
    _texturelist.filepath.push_back(filename);
    _texturelist.texturename.push_back(name);
    return true;
}

void Scene::debugShadowMap(float near_plane, float far_plane) {
    _lightCubeShader->use();
    _lightCubeShader->setFloat("near_plane", near_plane);
    _lightCubeShader->setFloat("far_plane", far_plane);
    glActiveTexture(GL_TEXTURE0);
    _depthmap->bind();

    unsigned int quadVAO = 0;
    unsigned int quadVBO;

    if (quadVAO == 0)
    {
        float quadVertices[] = {
            // positions        // texture Coords
            -1.0f,  1.0f, 0.0f, 0.0f, 1.0f,
            -1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
            1.0f,  1.0f, 0.0f, 1.0f, 1.0f,
            1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
        };
        // setup plane VAO
        glGenVertexArrays(1, &quadVAO);
        glGenBuffers(1, &quadVBO);
        glBindVertexArray(quadVAO);
        glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    }
    glBindVertexArray(quadVAO);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glBindVertexArray(0);
}

void Scene::drawList(){
    static int count = 0;
    count++;
    if(count>=(_serise.max+1)*20) count=0;
    switch(_ShadowRenderMode){
        case ShadowRenderMode::None:
        for(int i=0;i<_objectlist.ModelList.size();i++){
            if(!_objectlist.visible[i]) continue;
            if(series_flag&&i<_serise.sequence.size()&&_serise.max>-1&&_serise.sequence[i]!=-1){
                if (_serise.sequence[i]!=count/20) continue;
            }
            _pbrShader->use();
            const glm::mat4 projection = _camera->getProjectionMatrix();
            const glm::mat4 view = _camera->getViewMatrix();
            _pbrShader->setMat4("projection", projection);
            _pbrShader->setMat4("view", view);
            _pbrShader->setMat4("model", _objectlist.ModelList[i]->getModelMatrix());

            _pbrShader->setVec3("uLightPos", _directionlight->position);
            _pbrShader->setVec3("uCameraPos", _camera->position);
            _pbrShader->setVec3("uLightRadiance", _directionlight->radiance);
            _pbrShader->setFloat("ka", _directionlight->ka);

            _pbrShader->setFloat("uRoughness", _objectlist.roughness[i]);
            _pbrShader->setFloat("uMetallic", _objectlist.metallic[i]);
            if(_objectlist.color_flag[i]) _pbrShader->setVec3("uColor", _objectlist.Color[i]);
            else _pbrShader->setVec3("uColor", glm::vec3(1.0f));
            if(!_objectlist.color_flag[i] && _texturelist.texture[_objectlist.TextureIndex[i]] != nullptr){
                glActiveTexture(GL_TEXTURE0);
                _texturelist.texture[_objectlist.TextureIndex[i]]->bind();
                _objectlist.ModelList[i]->draw();
                _texturelist.texture[_objectlist.TextureIndex[i]]->unbind();
            }
            else _objectlist.ModelList[i]->draw();
        }
        break;

        case ShadowRenderMode::ShadowMapping: {
            glm::vec3 lightPos = _directionlight->position;
            glm::mat4 lightProjection, lightView, lightMatrix;
            float size = 50.0f;
            
            lightProjection = glm::ortho(-size, size, -size, size, 0.1f, 100.0f);
            // lightProjection =glm::perspective(glm::radians(45.0f), 1.0f,0.1f, 100.0f);
            // lightView = glm::lookAt(lightPos, lightPos + LightList[0]->getFront(), LightList[0]->getUp());
            lightView = glm::lookAt(lightPos, glm::vec3(0.0f), glm::vec3(0.0, 1.0, 0.0));;
            lightMatrix = lightProjection * lightView;

            // light pass
            _shadowShader->use();
            glViewport(0, 0, _shadowWidth, _shadowHeight);
            _depthfbo->bind();
            glClear(GL_DEPTH_BUFFER_BIT);
            
            _shadowShader->setMat4("uLightSpaceMatrix", lightMatrix);           
            for(int i = 0; i < _objectlist.ModelList.size(); i++){
                if(!_objectlist.visible[i]) continue;
                if(series_flag&&i<_serise.sequence.size()&&_serise.max>-1&&_serise.sequence[i]!=-1){
                    if (_serise.sequence[i]!=count/20) continue;
                }

                _shadowShader->setMat4("model", _objectlist.ModelList[i]->getModelMatrix());
                _objectlist.ModelList[i]->draw();
            }
            _depthfbo->unbind();
            
            // reset viewport
            glViewport(0, 0, _windowWidth, _windowHeight);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            // debugShadowMap(LightList[0]->near_plane, LightList[0]->far_plane);

            // camera pass
            for(int i = 0; i < _objectlist.ModelList.size(); i++){
                if(!_objectlist.visible[i]) continue;           
                _shadowMappingShader->use();
                if(!_objectlist.visible[i]) continue;
                if(series_flag&&i<_serise.sequence.size()&&_serise.max>-1&&_serise.sequence[i]!=-1){
                    if (_serise.sequence[i]!=count/20) continue;
                }

                _shadowMappingShader->setInt("uShadowMap", 0);
                _shadowMappingShader->setInt("uAlbedoMap", 1);
                glActiveTexture(GL_TEXTURE0);
                _depthmap->bind();

                const glm::mat4 projection = _camera->getProjectionMatrix();
                const glm::mat4 view = _camera->getViewMatrix();
                
                _shadowMappingShader->setMat4("projection", projection);
                _shadowMappingShader->setMat4("view", view);
                _shadowMappingShader->setMat4("model", _objectlist.ModelList[i]->getModelMatrix());
                _shadowMappingShader->setMat4("uLightSpaceMatrix", lightMatrix);

                _shadowMappingShader->setVec3("uLightPos", _directionlight->position);
                _shadowMappingShader->setVec3("uCameraPos", _camera->position);
                _shadowMappingShader->setVec3("uLightRadiance", _directionlight->radiance);
                _shadowMappingShader->setFloat("ka", _directionlight->ka);

                _shadowMappingShader->setFloat("uRoughness", _objectlist.roughness[i]);
                _shadowMappingShader->setFloat("uMetallic", _objectlist.metallic[i]);

                if(_objectlist.color_flag[i]) _shadowMappingShader->setVec3("uColor", _objectlist.Color[i]);
                else _shadowMappingShader->setVec3("uColor", glm::vec3(1.0f));
                if(!_objectlist.color_flag[i] && _texturelist.texture[_objectlist.TextureIndex[i]] != nullptr){
                    glActiveTexture(GL_TEXTURE1);
                    _texturelist.texture[_objectlist.TextureIndex[i]]->bind();
                    _objectlist.ModelList[i]->draw();
                    _texturelist.texture[_objectlist.TextureIndex[i]]->unbind();
                }
                else _objectlist.ModelList[i]->draw();
            }
            glActiveTexture(GL_TEXTURE0);
            _depthmap->unbind();
        }
        break;

        case ShadowRenderMode::PCF: {
            glm::vec3 lightPos = _directionlight->position;
            glm::mat4 lightProjection, lightView, lightMatrix;
            float size = 50.0f;
            
            lightProjection = glm::ortho(-size, size, -size, size, 0.1f, 100.0f);
            // lightProjection =glm::perspective(glm::radians(90.0f), 1.0f,0.1f, 100.0f);
            // lightView = glm::lookAt(lightPos, lightPos + LightList[0]->getFront(), LightList[0]->getUp());
            lightView = glm::lookAt(lightPos, glm::vec3(0.0f), glm::vec3(0.0, 1.0, 0.0));
            lightMatrix = lightProjection * lightView;

            // light pass
            _shadowShader->use();
            glViewport(0, 0, _shadowWidth, _shadowHeight);
            _depthfbo->bind();
            glClear(GL_DEPTH_BUFFER_BIT);
            
            _shadowShader->setMat4("uLightSpaceMatrix", lightMatrix);           
            for(int i = 0; i < _objectlist.ModelList.size(); i++){
                if(!_objectlist.visible[i]) continue;
                if(series_flag&&i<_serise.sequence.size()&&_serise.max>-1&&_serise.sequence[i]!=-1){
                    if (_serise.sequence[i]!=count/20) continue;
                }                

                _shadowShader->setMat4("model", _objectlist.ModelList[i]->getModelMatrix());
                _objectlist.ModelList[i]->draw();
            }
            _depthfbo->unbind();
            
            // reset viewport
            glViewport(0, 0, _windowWidth, _windowHeight);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            // camera pass
            for(int i = 0; i < _objectlist.ModelList.size(); i++){
                if(!_objectlist.visible[i]) continue;           
                if(series_flag && i<_serise.sequence.size() && _serise.max > -1 && _serise.sequence[i] != -1){
                    if (_serise.sequence[i]!=count/20) continue;
                }
                _pcfShader->use();

                _pcfShader->setInt("uShadowMap", 0);
                _pcfShader->setInt("uAlbedoMap", 1);
                glActiveTexture(GL_TEXTURE0);
                _depthmap->bind();

                const glm::mat4 projection = _camera->getProjectionMatrix();
                const glm::mat4 view = _camera->getViewMatrix();
                
                _pcfShader->setMat4("projection", projection);
                _pcfShader->setMat4("view", view);
                _pcfShader->setMat4("model", _objectlist.ModelList[i]->getModelMatrix());
                _pcfShader->setMat4("uLightSpaceMatrix", lightMatrix);

                _pcfShader->setVec3("uLightPos", _directionlight->position);
                _pcfShader->setVec3("uCameraPos", _camera->position);
                _pcfShader->setVec3("uLightRadiance", _directionlight->radiance);
                _pcfShader->setFloat("ka", _directionlight->ka);

                _pcfShader->setFloat("uRoughness", _objectlist.roughness[i]);
                _pcfShader->setFloat("uMetallic", _objectlist.metallic[i]);

                if(_objectlist.color_flag[i]) _pcfShader->setVec3("uColor", _objectlist.Color[i]);
                else _pcfShader->setVec3("uColor", glm::vec3(1.0f));
                if(!_objectlist.color_flag[i] && _texturelist.texture[_objectlist.TextureIndex[i]] != nullptr){
                    glActiveTexture(GL_TEXTURE1);
                    _texturelist.texture[_objectlist.TextureIndex[i]]->bind();
                    _objectlist.ModelList[i]->draw();
                    _texturelist.texture[_objectlist.TextureIndex[i]]->unbind();
                }
                else _objectlist.ModelList[i]->draw();
            }
            glActiveTexture(GL_TEXTURE0);
            _depthmap->unbind();
        }
        break;

        case ShadowRenderMode::PCSS: {
            glm::vec3 lightPos = _directionlight->position;
            glm::mat4 lightProjection, lightView, lightMatrix;
            float size = 50.0f;
            
            lightProjection = glm::ortho(-size, size, -size, size, 0.1f, 100.0f);
            //lightView = glm::lookAt(lightPos, lightPos + LightList[0]->getFront(), LightList[0]->getUp());
            lightView = glm::lookAt(lightPos, glm::vec3(0.0f), glm::vec3(0.0, 1.0, 0.0));
            lightMatrix = lightProjection * lightView;

            // light pass
            _shadowShader->use();
            glViewport(0, 0, _shadowWidth, _shadowHeight);
            _depthfbo->bind();
            glClear(GL_DEPTH_BUFFER_BIT);
            
            _shadowShader->setMat4("uLightSpaceMatrix", lightMatrix);           
            for(int i = 0; i < _objectlist.ModelList.size(); i++){
                if(!_objectlist.visible[i]) continue;
                if(series_flag && i<_serise.sequence.size() && _serise.max>-1 && _serise.sequence[i] != -1){
                    if (_serise.sequence[i] != count/20) continue;
                }
                _shadowShader->setMat4("model", _objectlist.ModelList[i]->getModelMatrix());
                _objectlist.ModelList[i]->draw();
            }
            _depthfbo->unbind();
            
            // reset viewport
            glViewport(0, 0, _windowWidth, _windowHeight);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            // camera pass
            for(int i = 0; i < _objectlist.ModelList.size(); i++){
                if(!_objectlist.visible[i]) continue;           
                _pcssShader->use();
                if(series_flag && i<_serise.sequence.size() && _serise.max>-1 && _serise.sequence[i]!=-1){
                    if (_serise.sequence[i] != count/20) continue;
                }
                _pcssShader->setInt("uShadowMap", 0);
                _pcssShader->setInt("uAlbedoMap", 1);
                glActiveTexture(GL_TEXTURE0);
                _depthmap->bind();

                const glm::mat4 projection = _camera->getProjectionMatrix();
                const glm::mat4 view = _camera->getViewMatrix();
                
                _pcssShader->setMat4("projection", projection);
                _pcssShader->setMat4("view", view);
                _pcssShader->setMat4("model", _objectlist.ModelList[i]->getModelMatrix());
                _pcssShader->setMat4("uLightSpaceMatrix", lightMatrix);

                _pcssShader->setVec3("uLightPos", _directionlight->position);
                _pcssShader->setVec3("uCameraPos", _camera->position);
                _pcssShader->setVec3("uLightRadiance", _directionlight->radiance);
                _pcssShader->setFloat("ka", _directionlight->ka);

                _pcssShader->setFloat("uRoughness", _objectlist.roughness[i]);
                _pcssShader->setFloat("uMetallic", _objectlist.metallic[i]);

                if(_objectlist.color_flag[i]) _pcssShader->setVec3("uColor", _objectlist.Color[i]);
                else _pcssShader->setVec3("uColor", glm::vec3(1.0f));
                if(!_objectlist.color_flag[i] && _texturelist.texture[_objectlist.TextureIndex[i]] != nullptr){
                    glActiveTexture(GL_TEXTURE1);
                    _texturelist.texture[_objectlist.TextureIndex[i]]->bind();
                    _objectlist.ModelList[i]->draw();
                    _texturelist.texture[_objectlist.TextureIndex[i]]->unbind();
                }
                else _objectlist.ModelList[i]->draw();
                glActiveTexture(GL_TEXTURE0);
                _depthmap->unbind();
            }
        }
        break;

        case ShadowRenderMode::SSR: {
            // pass1
            glm::vec3 lightPos = _directionlight->position;
            glm::mat4 lightProjection, lightView, lightMatrix;
            float size = 50.0f;
            
            lightProjection = glm::ortho(-size, size, -size, size, 0.1f, 100.0f);
            lightView = glm::lookAt(lightPos, glm::vec3(0.0f), glm::vec3(0.0, 1.0, 0.0));
            lightMatrix = lightProjection * lightView;

            _shadowShader->use();
            glViewport(0, 0, _shadowWidth, _shadowHeight);
            _depthfbo->bind();
            glClear(GL_DEPTH_BUFFER_BIT);
            
            _shadowShader->setMat4("uLightSpaceMatrix", lightMatrix);           
            for(int i = 0; i < _objectlist.ModelList.size(); i++){
                if(!_objectlist.visible[i]) continue;
                if(series_flag && i<_serise.sequence.size() && _serise.max>-1 && _serise.sequence[i] != -1){
                    if (_serise.sequence[i] != count/20) continue;
                }
                _shadowShader->setMat4("model", _objectlist.ModelList[i]->getModelMatrix());
                _objectlist.ModelList[i]->draw();
            }
            _depthfbo->unbind();
            
            // reset viewport
            glViewport(0, 0, _windowWidth, _windowHeight);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            // pass2
            _gbufferfbo->bind();
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            _gbufferShader->use();
            const glm::mat4 projection = _camera->getProjectionMatrix();
            const glm::mat4 view = _camera->getViewMatrix();
            _gbufferShader->setMat4("uProjectionMatrix", projection);
            _gbufferShader->setMat4("uViewMatrix", view);
            _gbufferShader->setMat4("uLightVP",lightMatrix);
            _gbufferShader->setVec3("uLightPos", _directionlight->position);
            _gbufferShader->setInt("uShadowMap",0);
            _gbufferShader->setInt("uAlbedoMap",1);
            glEnable(GL_TEXTURE0);
            _depthmap->bind();
            for(int i = 0; i < _objectlist.ModelList.size(); i++){
                if(!_objectlist.visible[i]) continue;
                if(series_flag && i<_serise.sequence.size() && _serise.max>-1 && _serise.sequence[i] != -1){
                    if (_serise.sequence[i] != count/20) continue;
                }
                _gbufferShader->setMat4("uModelMatrix", _objectlist.ModelList[i]->getModelMatrix());
                _gbufferShader->setFloat("roughness", _objectlist.roughness[i]);
                _gbufferShader->setFloat("metallic", _objectlist.metallic[i]);
                if(_objectlist.color_flag[i]) _gbufferShader->setVec3("uColor", _objectlist.Color[i]);
                else _gbufferShader->setVec3("uColor", glm::vec3(1.0f));
                if(!_objectlist.color_flag[i] && _texturelist.texture[_objectlist.TextureIndex[i]] != nullptr){
                    glActiveTexture(GL_TEXTURE1);
                    _texturelist.texture[_objectlist.TextureIndex[i]]->bind();
                    _objectlist.ModelList[i]->draw();
                    _texturelist.texture[_objectlist.TextureIndex[i]]->unbind();
                }
                else _objectlist.ModelList[i]->draw();
            }
            _gbufferfbo->unbind();
            glEnable(GL_TEXTURE0);
            _depthmap->unbind();

            // pass3
            _ssrShader->use();
            _ssrShader->setMat4("uProjectionMatrix", projection);
            _ssrShader->setMat4("uViewMatrix", view);
            _ssrShader->setVec3("uLightDir", _directionlight->position);
            _ssrShader->setVec3("uCameraPos", _camera->position);
            _ssrShader->setVec3("uLightRadiance", _directionlight->radiance);
            glActiveTexture(GL_TEXTURE0);_diffusetexuture->bind();
            glActiveTexture(GL_TEXTURE1);_depthtexture->bind();
            glActiveTexture(GL_TEXTURE2);_normaltexture->bind();
            glActiveTexture(GL_TEXTURE3);_visibilitytexture->bind();
            glActiveTexture(GL_TEXTURE4);_colortexture->bind();
            glActiveTexture(GL_TEXTURE5);_positiontexture->bind();
            _ssrShader->setInt("uGDiffuse",0);
            _ssrShader->setInt("uGDepth",1);
            _ssrShader->setInt("uGNormalWorld",2);
            _ssrShader->setInt("uGShadow",3);
            _ssrShader->setInt("uGColor",4);
            _ssrShader->setInt("uGPosition",5);
            _ssrShader->setInt("Width",_windowWidth);
            _ssrShader->setInt("Height",_windowHeight);
            _fullscrennquad->draw();
            _skybox->draw(projection, view);
            glActiveTexture(GL_TEXTURE0);_diffusetexuture->unbind();
            glActiveTexture(GL_TEXTURE1);_depthtexture->unbind();
            glActiveTexture(GL_TEXTURE2);_normaltexture->unbind();
            glActiveTexture(GL_TEXTURE3);_visibilitytexture->unbind();
            glActiveTexture(GL_TEXTURE4);_colortexture->unbind();
            glActiveTexture(GL_TEXTURE5);_positiontexture->unbind();
        }
        
        break;
    
        case ShadowRenderMode::SSR_Filter: {
            // pass1
            glm::vec3 lightPos = _directionlight->position;
            glm::mat4 lightProjection, lightView, lightMatrix;
            float size = 50.0f;
            
            lightProjection = glm::ortho(-size, size, -size, size, 0.1f, 100.0f);
            lightView = glm::lookAt(lightPos, glm::vec3(0.0f), glm::vec3(0.0, 1.0, 0.0));
            lightMatrix = lightProjection * lightView;

            _shadowShader->use();
            glViewport(0, 0, _shadowWidth, _shadowHeight);
            _depthfbo->bind();
            glClear(GL_DEPTH_BUFFER_BIT);
            
            _shadowShader->setMat4("uLightSpaceMatrix", lightMatrix);           
            for(int i = 0; i < _objectlist.ModelList.size(); i++){
                if(!_objectlist.visible[i]) continue;
                if(series_flag && i<_serise.sequence.size() && _serise.max>-1 && _serise.sequence[i] != -1){
                    if (_serise.sequence[i] != count/20) continue;
                }
                _shadowShader->setMat4("model", _objectlist.ModelList[i]->getModelMatrix());
                _objectlist.ModelList[i]->draw();
            }
            _depthfbo->unbind();
            
            // reset viewport
            glViewport(0, 0, _windowWidth, _windowHeight);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            // pass2
            _gbufferfbo->bind();
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            _gbufferShader->use();
            const glm::mat4 projection = _camera->getProjectionMatrix();
            const glm::mat4 view = _camera->getViewMatrix();
            _gbufferShader->setMat4("uProjectionMatrix", projection);
            _gbufferShader->setMat4("uViewMatrix", view);
            _gbufferShader->setMat4("uLightVP",lightMatrix);
            _gbufferShader->setVec3("uLightPos", _directionlight->position);
            _gbufferShader->setInt("uShadowMap",0);
            _gbufferShader->setInt("uAlbedoMap",1);
            glEnable(GL_TEXTURE0);
            _depthmap->bind();
            for(int i = 0; i < _objectlist.ModelList.size(); i++){
                if(!_objectlist.visible[i]) continue;
                if(series_flag && i<_serise.sequence.size() && _serise.max>-1 && _serise.sequence[i] != -1){
                    if (_serise.sequence[i] != count/20) continue;
                }
                _gbufferShader->setMat4("uModelMatrix", _objectlist.ModelList[i]->getModelMatrix());
                _gbufferShader->setFloat("roughness", _objectlist.roughness[i]);
                _gbufferShader->setFloat("metallic", _objectlist.metallic[i]);
                if(_objectlist.color_flag[i]) _gbufferShader->setVec3("uColor", _objectlist.Color[i]);
                else _gbufferShader->setVec3("uColor", glm::vec3(1.0f));
                if(!_objectlist.color_flag[i] && _texturelist.texture[_objectlist.TextureIndex[i]] != nullptr){
                    glActiveTexture(GL_TEXTURE1);
                    _texturelist.texture[_objectlist.TextureIndex[i]]->bind();
                    _objectlist.ModelList[i]->draw();
                    _texturelist.texture[_objectlist.TextureIndex[i]]->unbind();
                }
                else _objectlist.ModelList[i]->draw();
            }
            _gbufferfbo->unbind();
            glEnable(GL_TEXTURE0);
            _depthmap->unbind();

            // pass3
            _filterfbo->bind();
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            _ssrShader->use();
            _ssrShader->setMat4("uProjectionMatrix", projection);
            _ssrShader->setMat4("uViewMatrix", view);
            _ssrShader->setVec3("uLightDir", _directionlight->position);
            _ssrShader->setVec3("uCameraPos", _camera->position);
            _ssrShader->setVec3("uLightRadiance", _directionlight->radiance);
            glActiveTexture(GL_TEXTURE0);_diffusetexuture->bind();
            glActiveTexture(GL_TEXTURE1);_depthtexture->bind();
            glActiveTexture(GL_TEXTURE2);_normaltexture->bind();
            glActiveTexture(GL_TEXTURE3);_visibilitytexture->bind();
            glActiveTexture(GL_TEXTURE4);_colortexture->bind();
            glActiveTexture(GL_TEXTURE5);_positiontexture->bind();
            _ssrShader->setInt("uGDiffuse",0);
            _ssrShader->setInt("uGDepth",1);
            _ssrShader->setInt("uGNormalWorld",2);
            _ssrShader->setInt("uGShadow",3);
            _ssrShader->setInt("uGColor",4);
            _ssrShader->setInt("uGPosition",5);
            _ssrShader->setInt("Width",_windowWidth);
            _ssrShader->setInt("Height",_windowHeight);
            _fullscrennquad->draw();
            _skybox->draw(projection, view);
            
            glActiveTexture(GL_TEXTURE0);_diffusetexuture->unbind();
            glActiveTexture(GL_TEXTURE1);_depthtexture->unbind();
            glActiveTexture(GL_TEXTURE2);_normaltexture->unbind();
            glActiveTexture(GL_TEXTURE3);_visibilitytexture->unbind();
            glActiveTexture(GL_TEXTURE4);_colortexture->unbind();
            glActiveTexture(GL_TEXTURE5);_positiontexture->unbind();
            
            _filterfbo->unbind();

            // pass4
            _filterShader->use();
            glActiveTexture(GL_TEXTURE1);_normaltexture->bind();
            glActiveTexture(GL_TEXTURE2);_positiontexture->bind();
            glActiveTexture(GL_TEXTURE3);_beauty->bind();
            _filterShader->setInt("uGNormalWorld",1);
            _filterShader->setInt("uGPosition",2);
            _filterShader->setInt("Width",_windowWidth);
            _filterShader->setInt("Height",_windowHeight);
            _filterShader->setInt("uBeauty",3);


            _fullscrennquad->draw();


            glActiveTexture(GL_TEXTURE1);_normaltexture->unbind();
            glActiveTexture(GL_TEXTURE2);_positiontexture->unbind();
            glActiveTexture(GL_TEXTURE3);_beauty->unbind();
            glActiveTexture(GL_TEXTURE0);
        }
        break;

        case ShadowRenderMode::Path_Tracing: {
            PathTracing();
        }
    }
}

void Scene::PathTracing() {
    vec3 viewPos = vec3(_camera->position.x, _camera->position.y, _camera->position.z + 60.0f);
    mat4 cameraRotate = _camera->getViewMatrix();
    cameraRotate = inverse(cameraRotate);

    // 传 uniform 给 pass1
    glUseProgram(pass1.program);
    glUniform3fv(glGetUniformLocation(pass1.program, "uCameraPos"), 1, value_ptr(viewPos));
    glUniformMatrix4fv(glGetUniformLocation(pass1.program, "cameraRotate"), 1, GL_FALSE, value_ptr(cameraRotate));
    glUniform1ui(glGetUniformLocation(pass1.program, "frameCounter"), frameCounter++);
    glUniform1i(glGetUniformLocation(pass1.program, "hdrResolution"), hdrResolution); 

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_BUFFER, _trianglesTextureBuffer);
    glUniform1i(glGetUniformLocation(pass1.program, "triangles"), 0);

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_BUFFER, _nodesTextureBuffer);
    glUniform1i(glGetUniformLocation(pass1.program, "nodes"), 1);

    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, _lastFrame);
    glUniform1i(glGetUniformLocation(pass1.program, "lastFrame"), 2);

    glActiveTexture(GL_TEXTURE3);
    glBindTexture(GL_TEXTURE_2D, _hdrMap);
    glUniform1i(glGetUniformLocation(pass1.program, "hdrMap"), 3);

    glActiveTexture(GL_TEXTURE4);
    glBindTexture(GL_TEXTURE_2D, hdrCache);
    glUniform1i(glGetUniformLocation(pass1.program, "hdrCache"), 4);

    // 绘制
    pass1.draw();
    pass2.draw(pass1.colorAttachments);
    pass3.draw(pass2.colorAttachments);
}

void Scene::drawGUI()  {
    ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();
	const auto flags =
		ImGuiWindowFlags_AlwaysAutoResize |
		ImGuiWindowFlags_NoSavedSettings;
    static bool file_flags=false;
    static bool object_flags=false;
    static bool help_flags=false;
    static bool NURBS_flags=false;
    static bool wireframe = false;

    if (wireframe) {
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	} else {
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	}

	if (!ImGui::Begin("Main Window", nullptr, flags)) {
		ImGui::End();
	} else {
		ImGui::Text("This is our Main Window.");
        if(mouse_capture_flag)ImGui::Text("Use Alt to control mouse.");
        else ImGui::Text("Use Alt to control camera.");
        if(ImGui::Button("HELP")){
            help_flags=!help_flags;
        }
        ImGui::SameLine();
        ImGui::Text("Click this to view more details");
		ImGui::Separator();

        ImGui::Text("Check to open other windows");
        ImGui::Checkbox("File Control Window",&file_flags);
        ImGui::Checkbox("Object Control Window",&object_flags);
        ImGui::Checkbox("NURBS Control Window",&NURBS_flags);
        ImGui::Separator();

        ImGui::Text("Choose ShadowRenderMode");
        ImGui::RadioButton("None",(int *)&_ShadowRenderMode,(int)ShadowRenderMode::None);ImGui::SameLine();
        ImGui::RadioButton("ShadowMapping",(int *)&_ShadowRenderMode,(int)ShadowRenderMode::ShadowMapping);ImGui::SameLine();
        ImGui::RadioButton("PCF",(int *)&_ShadowRenderMode,(int)ShadowRenderMode::PCF);ImGui::SameLine();
        ImGui::RadioButton("PCSS",(int *)&_ShadowRenderMode,(int)ShadowRenderMode::PCSS);
        ImGui::RadioButton("SSR",(int *)&_ShadowRenderMode,(int)ShadowRenderMode::SSR);ImGui::SameLine();
        ImGui::RadioButton("SSR_Filter",(int *)&_ShadowRenderMode,(int)ShadowRenderMode::SSR_Filter);ImGui::SameLine();
        ImGui::RadioButton("Path_Tracing",(int *)&_ShadowRenderMode,(int)ShadowRenderMode::Path_Tracing);
        ImGui::Separator();

        ImGui::Text("Game Options");
        ImGui::Checkbox("Collision detect",&collision_flag);
        ImGui::RadioButton("FPS-style camera",(int *)&_CameraMode,(int)CameraMode::FPS);ImGui::SameLine();
        ImGui::RadioButton("Free camera",(int *)&_CameraMode,(int)CameraMode::Free);
        ImGui::Separator();

        ImGui::Text("Choose ScreenShotMode");ImGui::SameLine();
        ImGui::RadioButton("Normal",(int *)&_ScreenShotMode,(int)ScreenShotMode::Normal);ImGui::SameLine();
        ImGui::RadioButton("RayTracing",(int *)&_ScreenShotMode,(int)ScreenShotMode::RayTracing);
        if(ImGui::Button("Creat a ScreenShot")){
            //TODO::把接口塞进去
        }
        ImGui::Separator();
        ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
        ImGui::Checkbox("wireframe", &wireframe);
        ImGui::Checkbox("Draw series", &series_flag);
		ImGui::End();
	}
    if(help_flags)drawGUIhelp(help_flags);
    if(file_flags)drawGUIfile(file_flags);
    if(object_flags)drawGUIobj(object_flags);
    if(NURBS_flags){
        ImGui::Begin("NURBS Window", &NURBS_flags);
        ImGui::Text("In this Window, you can build NURBS surface with mouse click.");
        ImGui::End();
    }
	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void Scene::drawGUIfile(bool &flag){
    static char loadname[128] = "Type your file/object name here";
    static char filepath[128] = "Type your file path acurrately!";
    ImGui::Begin("File Window", &flag);
    ImGui::Text("In this Window, you can load OBJ files or textures with filepath.");
    ImGui::Text("You can still export all the scene to OBJ files");
    ImGui::NewLine();
    ImGui::Separator();
    ImGui::InputText("input file path", filepath, IM_ARRAYSIZE(filepath));
    ImGui::InputText("input file name", loadname, IM_ARRAYSIZE(loadname));
    if(ImGui::Button("Load OBJ file")){
        std::string filename =filepath;
        std::string name=loadname;
        if (!addModel(filename,loadname)) filename="Load Failed!";
        else filename="Load Success!";
        name=" ";
        strcpy(loadname,name.c_str());
        strcpy(filepath,filename.c_str());
    }
    ImGui::SameLine();
    if(ImGui::Button("Load Texture file")){
        std::string filename =filepath;
        std::string name=loadname;
        if(!addTexture(filename,loadname)) filename="Load Failed!";
        else filename="Load Success!";
        name=" ";
        strcpy(loadname,name.c_str());
        strcpy(filepath,filename.c_str());
    }
    ImGui::Separator();
    ImGui::Text("This will merge all the objects to a single group.");
    ImGui::NewLine();
    if(ImGui::Button("Export scene to OBJ")){
        // TODO::Export scene to OBJ
    }
    ImGui::Text("This will save scene information without objects");
    if(ImGui::Button("Export scene to CGF")){
        SceneSave();
    }
    ImGui::End();
}

void Scene::drawGUIobj(bool &flag){
    ImGui::Begin("Object Window", &flag);
    ImGui::Text("In this Window, you can view all the objects in the ");
    ImGui::Text("scene including light.You are free to set their scales, ");
    ImGui::Text("rotations and locations.For normal object, you can set ");
    ImGui::Text("roughness and color.For light, you can set intensity and color.");
    if(ImGui::CollapsingHeader("Add Fundmental Elements")){
        static float Cradius=1.0;
        static float Cheight=3.0;
        static float lenth=1.0;
        static float Sradius=1.0;
        static char loadname[128] = "Type your object name here";
        static int Psides=3;
        static float Pheight=3.0;
        static float Pradius=1.0;
        static int Fsides=3;
        static float Fheight=3.0;
        static float Fradius1=1.0;
        static float Fradius2=2.0;
        static float Coradius=1.0;
        static float Coheight=1.5;
        ImGui::InputText("input file name", loadname, IM_ARRAYSIZE(loadname));
        ImGui::NewLine();
        ImGui::SliderFloat("Cube Side Lenth",&lenth,0.1,10.0,"%.3f");
        if(ImGui::Button("Press to Create a Cube")){
            std::string name =loadname;
            AddCube(lenth/2,name);
            name="Creat Success!";
            strcpy(loadname,name.c_str());
        }
        ImGui::Separator();
        ImGui::SliderFloat("Sphere radius",&Sradius,0.1,10.0,"%.3f");
        if(ImGui::Button("Press to Create a Sphere")){
            std::string name =loadname;
            AddSphere(Sradius,name);
            name="Creat Success!";
            strcpy(loadname,name.c_str());
        }
        ImGui::Separator();
        ImGui::SliderFloat("Cylinder radius",&Cradius,0.1,10.0,"%.3f");
        ImGui::SliderFloat("Cylinder height",&Cheight,0.1,10.0,"%.3f");
        if(ImGui::Button("Press to Create a Cylinder")){
            std::string name =loadname;
            AddCylinder(Cradius,Cheight,name);
            name="Creat Success!";
            strcpy(loadname,name.c_str());
        }
        ImGui::Separator();
        ImGui::SliderFloat("Cone radius",&Coradius,0.1,10.0,"%.3f");
        ImGui::SliderFloat("Cone height",&Coheight,0.1,10.0,"%.3f");
        if(ImGui::Button("Press to Create a Cone")){
            std::string name =loadname;
            AddCone(Coradius,Coheight*2,name);
            name="Creat Success!";
            strcpy(loadname,name.c_str());
        }
        ImGui::Separator();
        ImGui::SliderInt("Prism  edges",&Psides,3,20);
        ImGui::SliderFloat("Prism radius",&Pradius,0.1,10.0,"%.3f");
        ImGui::SliderFloat("Prism height",&Pheight,0.1,10.0,"%.3f");
        if(ImGui::Button("Press to Create a Prism")){
            std::string name =loadname;
            AddPrism(Pradius,Pheight,name,Psides);
            name="Creat Success!";
            strcpy(loadname,name.c_str());
        }
        ImGui::Separator();
        ImGui::SliderInt("Frustum  edges",&Fsides,3,20);
        ImGui::SliderFloat("Frustum radius1",&Fradius1,0.1,10.0,"%.3f");
        ImGui::SliderFloat("Frustum radius2",&Fradius2,0.1,10.0,"%.3f");
        ImGui::SliderFloat("Frustum height",&Fheight,0.1,10.0,"%.3f");
        if(ImGui::Button("Press to Create a Frustum")){
            std::string name =loadname;
            AddFrustum(Fradius1,Fradius2,Fheight,name,Fsides);
            name="Creat Success!";
            strcpy(loadname,name.c_str());
        }
        ImGui::Separator();
    }
    if(ImGui::CollapsingHeader("Object List")){
        for (int i =0;i<_objectlist.ModelList.size();i++){
            if(ImGui::TreeNode((void*)(intptr_t)i,"object %s",_objectlist.objectname[i].c_str())){
                ImVec4 color = ImVec4(_objectlist.Color[i].x, _objectlist.Color[i].y, _objectlist.Color[i].z,1.0f);
                const float rota_angle=0.05f;
                float position[3]={_objectlist.ModelList[i]->position.x,_objectlist.ModelList[i]->position.y,_objectlist.ModelList[i]->position.z};
                float scale[3] = {_objectlist.ModelList[i]->scale.x,_objectlist.ModelList[i]->scale.y,_objectlist.ModelList[i]->scale.z};
                static char newname[128]="Type your new object name here";
                bool color_flag=_objectlist.color_flag[i];
                float roughness=_objectlist.roughness[i];
                float metallic=_objectlist.metallic[i];
                ImGui::Checkbox("Use color instead of texture",&color_flag);
                _objectlist.color_flag[i]=color_flag;
                ImGui::ColorEdit3("Object Color",(float*)&color);
                _objectlist.Color[i]=glm::vec3(color.x,color.y,color.z);
                ImGui::SliderFloat("Roughness",&roughness,0.0f,1.0f,"%.2f");
                _objectlist.roughness[i]=roughness;
                ImGui::SliderFloat("Metallic",&metallic,0.0f,1.0f,"%.2f");
                _objectlist.metallic[i]=metallic;
                ImGui::DragFloat3("Scale",scale,0.005f,0.0f,10.0f,"%.3f");ImGui::SameLine();
                if(ImGui::Button("Proportional base scale.x")){
                    scale[2]=scale[1]=scale[0];  
                }
                ImGui::DragFloat3("Position",position,0.005f,-100.0f,100.0f,"%.3f");
                _objectlist.ModelList[i]->scale=glm::vec3{scale[0],scale[1],scale[2]};
                _objectlist.ModelList[i]->position=glm::vec3{position[0],position[1],position[2]};
                float spacing = ImGui::GetStyle().ItemInnerSpacing.x;
                ImGui::PushButtonRepeat(true);
                ImGui::Text("Hode to rotate:   ");ImGui::SameLine();
                ImGui::Text("rotation-x");ImGui::SameLine();
                if (ImGui::ArrowButton("##left", ImGuiDir_Left)) {
                    _objectlist.ModelList[i]->rotation =glm::normalize(glm::quat{ cos(rota_angle / 2),0.0,1.0*sin(rota_angle / 2),0.0 }*_objectlist.ModelList[i]->rotation);
                }
                ImGui::SameLine(0.0f, spacing);
                if (ImGui::ArrowButton("##right", ImGuiDir_Right)) {
                    _objectlist.ModelList[i]->rotation =glm::normalize(glm::quat{ cos(rota_angle / 2),0.0,-1.0*sin(rota_angle / 2),0.0 }*_objectlist.ModelList[i]->rotation);
                }
                ImGui::SameLine();ImGui::Text("rotation-y");ImGui::SameLine();
                if (ImGui::ArrowButton("##left", ImGuiDir_Left)) {
                    _objectlist.ModelList[i]->rotation =glm::normalize(glm::quat{ cos(rota_angle / 2),1.0*sin(rota_angle / 2),0.0,0.0 }*_objectlist.ModelList[i]->rotation);
                }
                ImGui::SameLine(0.0f, spacing);
                if (ImGui::ArrowButton("##right", ImGuiDir_Right)) {
                    _objectlist.ModelList[i]->rotation =glm::normalize(glm::quat{ cos(rota_angle / 2),-1.0*sin(rota_angle / 2),0.0,0.0 }*_objectlist.ModelList[i]->rotation);
                }
                ImGui::SameLine();ImGui::Text("rotation-z");ImGui::SameLine();
                if (ImGui::ArrowButton("##left", ImGuiDir_Left)) {
                    _objectlist.ModelList[i]->rotation =glm::normalize(glm::quat{ cos(rota_angle / 2),0.0,0.0,1.0*sin(rota_angle / 2) }*_objectlist.ModelList[i]->rotation);
                }
                ImGui::SameLine(0.0f, spacing);
                if (ImGui::ArrowButton("##right", ImGuiDir_Right)) {
                    _objectlist.ModelList[i]->rotation =glm::normalize(glm::quat{ cos(rota_angle / 2),0.0,0.0,-1.0*sin(rota_angle / 2) }*_objectlist.ModelList[i]->rotation);
                }
                ImGui::PopButtonRepeat();
                if(!_objectlist.visible[i]&&ImGui::Button("Click me to display this object")) _objectlist.visible[i]=true;
                else if(_objectlist.visible[i]&&ImGui::Button("Click me to hide this object")) _objectlist.visible[i]=false;
                int texture_current_idx = _objectlist.TextureIndex[i]; // Here we store our selection data as an index.
                ImGui::Text("Click to choose texture for object");
                if (ImGui::BeginListBox("TextureList"))
                {
                    for (int n = 0; n < _texturelist.texturename.size(); n++)
                    {
                        const bool is_selected = (texture_current_idx == n);
                        if (ImGui::Selectable(_texturelist.texturename[n].c_str(), is_selected)){
                            texture_current_idx = n;
                            _objectlist.TextureIndex[i]=texture_current_idx;
                        }
                        if (is_selected)
                            ImGui::SetItemDefaultFocus();
                    }
                    ImGui::EndListBox();
                }
                if(ImGui::Button("Export this orginal model to filename.obj")) 
                    exportOBJ(_objectlist.ModelList[i]->_vertices,_objectlist.ModelList[i]->_indices,_objectlist.objectname[i]);
                if(ImGui::Button("Export transformed model to filename.obj")) 
                    exportTransOBJ(_objectlist.ModelList[i]->_vertices,_objectlist.ModelList[i]->_indices,_objectlist.objectname[i],_objectlist.ModelList[i]->getModelMatrix());
                if(_objectlist.ModelList.size()>1&&ImGui::Button("Delete this object")) deleteModel(i);
                ImGui::InputText("Change object name", newname, IM_ARRAYSIZE(newname));ImGui::SameLine();
                if(ImGui::Button("Enter name")){
                    std::string name=newname;
                    _objectlist.objectname[i]=name;
                    name="Change Success!";
                    name=" ";
                    strcpy(newname,name.c_str());
                }
                ImGui::TreePop();
            }
        }
    }
    if(ImGui::CollapsingHeader("Draw Series")){
        if(ImGui::Button("Init Series")){
            _serise.sequence.clear();
            for (int i = 0; i < _objectlist.ModelList.size(); i++){
                _serise.sequence.push_back(-1);
            }
        }
        _serise.max=-1;
        for(int i=0;i<_serise.sequence.size();i++){
            ImGui::SliderInt(_objectlist.objectname[i].c_str(),&_serise.sequence[i],-1,20);
            if(_serise.sequence[i]>_serise.max) _serise.max=_serise.sequence[i];
        }
    }
    if(ImGui::CollapsingHeader("Light Status")){
        ImVec4 color = ImVec4(_directionlight->color.x, _directionlight->color.y, _directionlight->color.z,1.0f);
        static float position[3]={_directionlight->position.x,_directionlight->position.y,_directionlight->position.z};
        ImGui::DragFloat3("light position",position,0.05f,-10.0f,10.0f,"%.3f");
        _directionlight->position=glm::vec3(position[0],position[1],position[2]);
        ImGui::DragFloat("Light Intensity",&_directionlight->intensity,0.05f,0.1f,10.0f,"%.3f");
        ImGui::ColorEdit3("Light Color",(float*)&color);
        _directionlight->color=glm::vec3(color.x,color.y,color.z);
        _directionlight->radiance=_directionlight->intensity*_directionlight->color;
    }
    
    ImGui::End();
}

void Scene::drawGUIhelp(bool &flag){
    ImGui::Begin("Helps", &flag);
    ImGui::Text("This is something could give you a help.");
    if (ImGui::Button("Close because I know how to use."))
        flag = false;
    ImGui::End();
}