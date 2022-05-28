#pragma once

#include <memory>
#include <string>

#include "./base/application.h"
#include "./base/model.h"
#include "./base/light.h"
#include "./base/glsl_program.h"
#include "./base/texture.h"
#include "./base/camera.h"
#include "./base/skybox.h"

enum class RenderMode {
	Normal
};

struct Material {
	std::shared_ptr<Texture2D> mapKd;
};


class Test : public Application {
public:
	Test(const Options& options);
	
	~Test();

private:
	std::unique_ptr<Model> _sphere;

	std::unique_ptr<Material> _Material;

	std::unique_ptr<PerspectiveCamera> _camera;
	std::unique_ptr<DirectionalLight> _light;

	std::unique_ptr<GLSLProgram> _Shader;

	std::unique_ptr<SkyBox> _skybox;

	enum RenderMode _renderMode = RenderMode::Normal;

	void initShader();

	void handleInput() override;

	void renderFrame() override;

	void userControl();
};