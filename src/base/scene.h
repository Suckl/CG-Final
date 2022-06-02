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

enum class ShadowRenderMode {
	None,ShadowMapping, PCSS, SSDO
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

class Scene:public Application{
public:
    Scene(const Options& options);
	~Scene();
private:
    // 物体清单，分别录入了模型纹理以及对应的Shader
    ObjectList _objectlist;
    TextureList _texturelist;
    
    std::vector<std::unique_ptr<Light>> LightList;
    std::unique_ptr<PerspectiveCamera> _camera;
    std::unique_ptr<SkyBox> _skybox;

    std::shared_ptr<GLSLProgram> _pbrShader;
    std::shared_ptr<GLSLProgram> _shadowShader;
    std::shared_ptr<GLSLProgram> _shadowMappingShader;
    std::shared_ptr<GLSLProgram> _pcssShader;
    std::shared_ptr<GLSLProgram> _lightCubeShader;

    enum ShadowRenderMode _ShadowRenderMode=ShadowRenderMode::None;
    enum ScreenShotMode _ScreenShotMode=ScreenShotMode::Normal;
    enum CameraMode _CameraMode=CameraMode::FPS;

    bool collision_flag=false;
    bool mouse_capture_flag=false;
    const float _cameraMoveSpeed = 10.0f;
    const float cameraRotateSpeed = 0.1f;

    const unsigned int _shadowWidth = 1024, _shadowHeight = 1024;
    unsigned int depthMapFBO;
    unsigned int depthMap;

    void initPBRShader();
    void initShadowShader();
    void initShadowMappingShader();
    void initPcssShader();
    void initLightCubeShader();

    void drawList();
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
    void SceneLoad();
};