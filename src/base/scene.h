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
    std::vector<std::unique_ptr<Model>> ModelList;
    std::vector<std::shared_ptr<Texture2D>> Texture2DList;
    // 针对每个物体单独设置Shader，现在看来意义不大
    // std::vector<std::shared_ptr<GLSLProgram>> ShaderList;
};

class Scene:public Application{
public:
    Scene(const Options& options);
	~Scene();
private:
    // 物体清单，分别录入了模型纹理以及对应的Shader
    ObjectList _objectlist;
    std::vector<std::unique_ptr<Light>> LightList;
    // 储存了所有类型的Shader，如果之后创建了许多Shader的话可以用enum来从里面取，现在还没有
    // std::vector<std::shared_ptr<GLSLProgram>> AllShader;
    // 测试用的Shader
    std::shared_ptr<GLSLProgram> _simpleShader;
    std::unique_ptr<PerspectiveCamera> _camera;
    enum ShadowRenderMode _ShadowRenderMode=ShadowRenderMode::None;
    enum ScreenShotMode _ScreenShotMode=ScreenShotMode::Normal;
    enum CameraMode _CameraMode=CameraMode::FPS;
    bool collision_flag=false;
    bool mouse_capture_flag=false;
    const float _cameraMoveSpeed = 10.0f;
    const float cameraRotateSpeed = 0.1f;

    void initSimpleShader();
    void drawList();
    // void addModel() const;
    // void export() const;
    void drawGUI() ;
    void handleInput() override;
	void renderFrame() override;
};