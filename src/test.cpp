#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include "test.h"

const std::string modelPath = "C:/Users/Hiyori/Documents/GitHub/CG-Final/build/media/bunny.obj";

const std::string earthTexturePath = "../media/earthmap.jpg";
const std::string planetTexturePath = "../media/planet_Quom1200.png";

const std::vector<std::string> skyboxTexturePaths = {
	"./../media/starfield/Right_Tex.jpg",
	"./../media/starfield/Left_Tex.jpg",
	"./../media/starfield/Up_Tex.jpg",
	"./../media/starfield/Down_Tex.jpg",
	"./../media/starfield/Front_Tex.jpg",
	"./../media/starfield/Back_Tex.jpg"
};

Test::Test(const Options& options): Application(options) {
	// init model
	_sphere.reset(new Model(modelPath));
	_sphere->scale = glm::vec3(1.0f, 1.0f, 1.0f);

	// init textures
	std::shared_ptr<Texture2D> earthTexture = std::make_shared<Texture2D>(earthTexturePath);
	std::shared_ptr<Texture2D> planetTexture = std::make_shared<Texture2D>(planetTexturePath);

	// init materials
	_Material.reset(new Material);
	_Material->mapKd = planetTexture;

	// init skybox
	_skybox.reset(new SkyBox(skyboxTexturePaths));

	// init camera
	_camera.reset(new PerspectiveCamera(
		glm::radians(50.0f), 1.0f * _windowWidth / _windowHeight, 0.1f, 10000.0f));
	_camera->position.z = 10.0f;

	// init light
	_light.reset(new DirectionalLight());
	_light->rotation = glm::angleAxis(glm::radians(45.0f), -glm::vec3(1.0f, 1.0f, 1.0f));

	// init shaders
	initShader();

	// init imgui
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGui::StyleColorsDark();
	ImGui_ImplGlfw_InitForOpenGL(_window, true);
	ImGui_ImplOpenGL3_Init();
}

Test::~Test() {
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();
}

void Test::initShader() {
	const char* vsCode =
		"#version 330 core\n"
		"layout(location = 0) in vec3 aPosition;\n"
		"layout(location = 1) in vec3 aNormal;\n"
		"layout(location = 2) in vec2 aTexCoord;\n"
		"out vec3 fPosition;\n"
		"out vec3 fNormal;\n"
		"out vec2 fTexCoord;\n"
		"uniform mat4 projection;\n"
		"uniform mat4 view;\n"
		"uniform mat4 model;\n"

		"void main() {\n"
		"	fPosition = vec3(model * vec4(aPosition, 1.0f));\n"
		"	fNormal = mat3(transpose(inverse(model))) * aNormal;\n"
		"	fTexCoord = aTexCoord;\n"
		"	gl_Position = projection * view * model * vec4(aPosition, 1.0f);\n"
		"}\n";

	const char* fsCode =
		"#version 330 core\n"
		"in vec3 fPosition;\n"
		"in vec3 fNormal;\n"
		"in vec2 fTexCoord;\n"
		"out vec4 color;\n"

		"struct DirectionalLight {\n"
		"	vec3 direction;\n"
		"	vec3 color;\n"
		"	float intensity;\n"
		"};\n"

		"struct Material {\n"
		"	vec3 Kd;\n"
		"};\n"

		"uniform Material material;\n"
		"uniform DirectionalLight light;\n"
		"uniform sampler2D mapKd;\n"

		"vec3 calcDirectionalLight(vec3 normal) {\n"
		"	vec3 lightDir = normalize(-light.direction);\n"
		"	vec3 diffuse = light.color * max(dot(lightDir, normal), 0.0f);\n"
		"	return light.intensity * diffuse;\n"
		"}\n"

		"void main() {\n"
		"	vec3 normal = normalize(fNormal);\n"
		"	vec3 color0 = calcDirectionalLight(normal);\n"
		"	color = vec4(color0, 1.0f);\n"
		//"	color = texture(mapKd, fTexCoord);\n"
		"}\n";

	_Shader.reset(new GLSLProgram);
	_Shader->attachVertexShader(vsCode);
	_Shader->attachFragmentShader(fsCode);
	_Shader->link();
}

void Test::handleInput() {
	const float angluarVelocity = 0.1f;
	const float angle = angluarVelocity * static_cast<float>(_deltaTime);
	const glm::vec3 axis = glm::vec3(0.0f, 1.0f, 0.0f);
	_sphere->rotation = glm::angleAxis(angle, axis) * _sphere->rotation;

	userControl();
}

void Test::renderFrame() {
	// some options related to imGUI
	static bool wireframe = false;
	
	// trivial things
	showFpsInWindowTitle();

	glClearColor(_clearColor.r, _clearColor.g, _clearColor.b, _clearColor.a);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glEnable(GL_DEPTH_TEST);

	if (wireframe) {
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	} else {
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	}

	const glm::mat4 projection = _camera->getProjectionMatrix();
	const glm::mat4 view = _camera->getViewMatrix();

	// draw planet
	switch (_renderMode) {
	case RenderMode::Normal:
		// 1. use the shader
		_Shader->use();

		// 2. transfer mvp matrices to gpu 
		_Shader->setMat4("projection", projection);
		_Shader->setMat4("view", view);
		_Shader->setMat4("model", _sphere->getModelMatrix());

		// // 3. transfer light attributes to gpu
		_Shader->setVec3("light.direction", _light->getFront());
		_Shader->setVec3("light.color", _light->color);
		_Shader->setFloat("light.intensity", _light->intensity);

		// 4. transfer materials to gpu
		_Material->mapKd->bind();

		break;
	}

	_sphere->draw();

	// draw skybox
	_skybox->draw(projection, view);

	// draw ui elements
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();

	const auto flags =
		ImGuiWindowFlags_AlwaysAutoResize |
		ImGuiWindowFlags_NoSavedSettings;

	if (!ImGui::Begin("Control Panel", nullptr, flags)) {
		ImGui::End();
	} else {
		ImGui::Text("Render Mode");
		ImGui::Separator();

		ImGui::RadioButton("Blend Texture Shading", (int*)&_renderMode, (int)(RenderMode::Normal));
		ImGui::NewLine();

		ImGui::Text("Directional light");
		ImGui::Separator();
		ImGui::SliderFloat("intensity", &_light->intensity, 0.0f, 2.0f);
		ImGui::ColorEdit3("color", (float*)&_light->color);
		ImGui::NewLine();

		ImGui::End();
	}

	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void Test::userControl()
{
	constexpr float cameraMoveSpeed = 5.0f;

	if (_keyboardInput.keyStates[GLFW_KEY_W] != GLFW_RELEASE) {
		_camera->position += _camera->getFront() * cameraMoveSpeed * _deltaTime;
	}

	if (_keyboardInput.keyStates[GLFW_KEY_A] != GLFW_RELEASE) {
		_camera->position -= _camera->getRight() * cameraMoveSpeed * _deltaTime;
	}

	if (_keyboardInput.keyStates[GLFW_KEY_S] != GLFW_RELEASE) {
		_camera->position -= _camera->getFront() * cameraMoveSpeed * _deltaTime;
	}

	if (_keyboardInput.keyStates[GLFW_KEY_D] != GLFW_RELEASE) {
		_camera->position += _camera->getRight() * cameraMoveSpeed * _deltaTime;
	}
}