#pragma once
#include"model.h"
#include"texture.h"
#include"light.h"
#include"camera.h"
#include"skybox.h"
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include"application.h"
#include"framebuffer.h"
#include"fullscreen_quad.h"

#include "PathTracingResources.h"

enum class ShadowRenderMode {
	None,ShadowMapping, PCF, PCSS, SSR, SSR_Filter, Path_Tracing
};

enum class ScreenShotMode {
	Normal, RayTracing
};

enum class CameraMode {
	FPS, Free
};

struct ObjectList{
    std::vector<std::string> objectname;//物体名称
    std::vector<bool> visible;//可见性
    std::vector<std::unique_ptr<Model>> ModelList;//物体模型
    std::vector<bool> color_flag;//应用纹理还是颜色
    std::vector<int> TextureIndex;//纹理下标
    std::vector<glm::vec3> Color;//颜色坐标
    std::vector<float> roughness;//粗糙度
    std::vector<float> metallic;//金属度
    std::vector<std::string> filepath;//保存用
};

struct TextureList{
    std::vector<std::shared_ptr<Texture2D>> texture;
    std::vector<std::string> texturename;
    std::vector<std::string> filepath;
};

struct Serise{
    std::vector<int> sequence;
    int max;
};

class Scene:public Application{
public:
    Scene(const Options& options);
	~Scene();
private:
    // 物体清单，分别录入了模型纹理以及对应的Shader
    ObjectList _objectlist;
    TextureList _texturelist;

    std::unique_ptr<DirectionalLight> _directionlight;
    std::unique_ptr<PerspectiveCamera> _camera;
    std::unique_ptr<SkyBox> _skybox;
    Serise _serise;//如果是-1，表示这个物体照常被绘制。如果是其他值i，表示这个物体在顺序i时被绘制

    std::shared_ptr<GLSLProgram> _pbrShader;
    std::shared_ptr<GLSLProgram> _shadowShader;
    std::shared_ptr<GLSLProgram> _shadowMappingShader;
    std::shared_ptr<GLSLProgram> _pcfShader;
    std::shared_ptr<GLSLProgram> _pcssShader;

    std::shared_ptr<GLSLProgram> _pointShadowShader;
    std::shared_ptr<GLSLProgram> _omnidirectionalShader;
    std::shared_ptr<GLSLProgram> _lightCubeShader;

    std::shared_ptr<GLSLProgram> _gbufferShader;
    std::shared_ptr<GLSLProgram> _ssrShader;
    std::shared_ptr<GLSLProgram> _filterShader;

    // SSR resources
    std::unique_ptr<Framebuffer> _depthfbo;
    std::unique_ptr<Framebuffer> _gbufferfbo;
    std::unique_ptr<Framebuffer> _filterfbo;

    std::unique_ptr<DataTexture> _depthmap;
    std::unique_ptr<DataTexture> _depthgbuffer;
    std::unique_ptr<DataTexture> _depthfilter;
    std::unique_ptr<DataTexture> _normaltexture;
    std::unique_ptr<DataTexture> _visibilitytexture;
    std::unique_ptr<DataTexture> _colortexture;
    std::unique_ptr<DataTexture> _diffusetexuture;
    std::unique_ptr<DataTexture> _depthtexture;
    std::unique_ptr<DataTexture> _positiontexture;
    std::unique_ptr<DataTexture> _beauty;
    std::unique_ptr<FullscreenQuad> _fullscrennquad;

    // Path Tracing resources
    std::vector<Triangle> triangles;
    BVHNode bvhNode;
    std::vector<Triangle_encoded> triangles_encoded;
    std::vector<BVHNode_encoded> nodes_encoded;

    GLuint _trianglesTextureBuffer;
    GLuint _nodesTextureBuffer;
    GLuint _lastFrame;

    RenderPass pass1;
    RenderPass pass2;
    RenderPass pass3;

    std::unique_ptr<Framebuffer> _pathTracingfbo;
    std::unique_ptr<Framebuffer> _pass2fbo;
    std::unique_ptr<Framebuffer> _pass3fbo;

    clock_t t1, t2;
    double dt, fps;
    unsigned int frameCounter = 0;

    enum ShadowRenderMode _ShadowRenderMode=ShadowRenderMode::None;
    enum ScreenShotMode _ScreenShotMode=ScreenShotMode::Normal;
    enum CameraMode _CameraMode=CameraMode::FPS;

    bool collision_flag=false;
    bool mouse_capture_flag=false;
    bool series_flag=false;
    const float _cameraMoveSpeed = 10.0f;
    const float cameraRotateSpeed = 0.1f;

    const unsigned int _shadowWidth = 2048, _shadowHeight = 2048;

    void initShader();
    void initPathTracingResources();
    void initPathTracingModel(int index, Material m);

    void drawList();
    void debugShadowMap(float near_plane, float far_plane);
    bool addTexture(const std::string filename,const std::string name);
    bool addModel(const std::string filename,const std::string name);

    // void export();
    void drawGUI();
    void drawGUIfile(bool &flag);
    void drawGUIobj(bool &flag);
    void drawGUIhelp(bool &flag);
    void handleInput() override;
	void renderFrame() override;
    void SceneSave();
    bool SceneLoad();
    void deleteModel(int index);
    void exportOBJ(const std::vector<Vertex> _vertices,const std::vector<uint32_t> _indices,std::string filename);
    void exportTransOBJ(const std::vector<Vertex> _vertices,const std::vector<uint32_t> _indices,std::string filename,glm::mat4 model);
    void AddCube(float l,std::string name);
    void AddSphere(float r,std::string name);
    void AddCylinder(float r,float h,std::string name);
    void AddPrism(float r,float h,std::string name,int sides);
    void AddFrustum(float r1,float r2,float h,std::string name,int sides);
    void AddCone(float r,float h,std::string name);

    void PathTracing();
};
