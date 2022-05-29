
#include"scene.h"

const std::string modelPath = "../media/sphere.obj";
const std::string earthTexturePath = "../media/earthmap.jpg";
const std::string planetTexturePath = "../media/planet_Quom1200.png";

Scene::Scene(const Options& options): Application(options) {

    
	// init model
    _objectlist.ModelList.push_back(nullptr);
    _objectlist.ModelList[0].reset(new Model(modelPath));
    _objectlist.ModelList[0]->scale=glm::vec3(5.0f,5.0f,5.0f);

	_objectlist.ModelList.push_back(nullptr);
    _objectlist.ModelList[1].reset(new Model(modelPath));
	_objectlist.ModelList[1]->scale = glm::vec3(3.0f, 3.0f, 3.0f);

    

	// init materials
    _objectlist.Texture2DList.push_back(nullptr);
	_objectlist.Texture2DList[0].reset(new Texture2D(earthTexturePath));

    _objectlist.Texture2DList.push_back(nullptr);
	_objectlist.Texture2DList[1].reset(new Texture2D(planetTexturePath));


	// init camera
	_camera.reset(new PerspectiveCamera(
		glm::radians(50.0f), 1.0f * _windowWidth / _windowHeight, 0.1f, 10000.0f));
	_camera->position.z = 10.0f;

	// init light
    LightList.push_back(nullptr);
	LightList[0].reset(new DirectionalLight());
	LightList[0]->rotation = glm::angleAxis(glm::radians(45.0f), -glm::vec3(1.0f, 1.0f, 1.0f));
	// init shaders
	initSimpleShader();
	// init imgui
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGui::StyleColorsDark();
	ImGui_ImplGlfw_InitForOpenGL(_window, true);
	ImGui_ImplOpenGL3_Init();
}

Scene::~Scene() {
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();
}

void Scene::initSimpleShader() {
	const char* vsCode =
		"#version 330 core\n"
		"layout(location = 0) in vec3 aPosition;\n"
		"layout(location = 1) in vec3 aNormal;\n"
		"layout(location = 2) in vec2 aTexCoord;\n"
		"out vec2 fTexCoord;\n"
		"uniform mat4 projection;\n"
		"uniform mat4 view;\n"
		"uniform mat4 model;\n"

		"void main() {\n"
		"	fTexCoord = aTexCoord;\n"
		"	gl_Position = projection * view * model * vec4(aPosition, 1.0f);\n"
		"}\n";

	const char* fsCode =
		"#version 330 core\n"
		"in vec2 fTexCoord;\n"
		"out vec4 color;\n"
		"uniform sampler2D mapKd;\n"
		"void main() {\n"
		"	color = texture(mapKd, fTexCoord);\n"
		// "	color=vec4(1.0);"
		"}\n";

	_simpleShader.reset(new GLSLProgram); 
	_simpleShader->attachVertexShader(vsCode);
	_simpleShader->attachFragmentShader(fsCode);
	_simpleShader->link();
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
		if(_CameraMode==CameraMode::Free)_camera->position += _camera->getFront() * _cameraMoveSpeed * (float)_deltaTime;
        else _camera->position += 
            glm::normalize(glm::vec3(_camera->getFront().x,0.0f,_camera->getFront().z)) * _cameraMoveSpeed * (float)_deltaTime;
	}

	if (_keyboardInput.keyStates[GLFW_KEY_A] != GLFW_RELEASE&&mouse_capture_flag) {
		if(_CameraMode==CameraMode::Free)_camera->position -= _camera->getRight() * _cameraMoveSpeed * (float)_deltaTime;
        else _camera->position -= 
            glm::normalize(glm::vec3(_camera->getRight().x,0.0f,_camera->getRight().z)) * _cameraMoveSpeed * (float)_deltaTime;
	}

	if (_keyboardInput.keyStates[GLFW_KEY_S] != GLFW_RELEASE&&mouse_capture_flag) {
		if(_CameraMode==CameraMode::Free)_camera->position -= _camera->getFront() * _cameraMoveSpeed * (float)_deltaTime;
        else _camera->position -= 
            glm::normalize(glm::vec3(_camera->getFront().x,0.0f,_camera->getFront().z)) * _cameraMoveSpeed * (float)_deltaTime;
	}

	if (_keyboardInput.keyStates[GLFW_KEY_D] != GLFW_RELEASE&&mouse_capture_flag) {
		if(_CameraMode==CameraMode::Free)_camera->position += _camera->getRight() * _cameraMoveSpeed * (float)_deltaTime;
        else _camera->position += 
            glm::normalize(glm::vec3(_camera->getRight().x,0.0f,_camera->getRight().z)) * _cameraMoveSpeed * (float)_deltaTime;
	}

    if (_keyboardInput.keyStates[GLFW_KEY_LEFT_ALT] != GLFW_RELEASE&&test==false) {
        test=true;
		mouse_capture_flag=!mouse_capture_flag;
        if(mouse_capture_flag) {glfwSetInputMode(_window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);printf("True");}
        else {glfwSetInputMode(_window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);printf("flase");}
	}
    if(_keyboardInput.keyStates[GLFW_KEY_LEFT_ALT] == GLFW_RELEASE&&test==true){
        test=false;
    }

    if (_mouseInput.move.xCurrent != _mouseInput.move.xOld&&mouse_capture_flag) {
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
		// std::cout << "mouse move in y direction" << std::endl;
		/* write your code here */
		// rotate around local right
        if(test) _mouseInput.move.yOld=_mouseInput.move.yCurrent;
		const float deltaY = static_cast<float>(_mouseInput.move.yCurrent - _mouseInput.move.yOld);
		const float angle = -cameraRotateSpeed * _deltaTime * deltaY;
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
	// draw 
	drawList();
    drawGUI();
}

// 之后还需要根据ShadowMode分别设置，现在只是简化
void Scene::drawList(){
    switch(_ShadowRenderMode){
        case ShadowRenderMode::None:
        for(int i=0;i<_objectlist.ModelList.size();i++){
            _simpleShader->use();
            const glm::mat4 projection = _camera->getProjectionMatrix();
            const glm::mat4 view = _camera->getViewMatrix();
            _simpleShader->setMat4("projection", projection);
            _simpleShader->setMat4("view", view);
            _simpleShader->setMat4("model", _objectlist.ModelList[i]->getModelMatrix());
            glActiveTexture(GL_TEXTURE0);
            _objectlist.Texture2DList[i]->bind();
            _objectlist.ModelList[i]->draw();
        }
        break;
    }
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
        ImGui::RadioButton("PCSS",(int *)&_ShadowRenderMode,(int)ShadowRenderMode::PCSS);ImGui::SameLine();
        ImGui::RadioButton("SSDO",(int *)&_ShadowRenderMode,(int)ShadowRenderMode::SSDO);
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
		ImGui::End();
	}
    if(help_flags){
        ImGui::Begin("Helps", &help_flags);
        ImGui::Text("This is something could give you a help.");
        if (ImGui::Button("Close because I know how to use."))
            help_flags = false;
        ImGui::End();
    }
    if(file_flags){
        static char filepath[128] = "Type your file path acurrately!";
        ImGui::Begin("File Window", &file_flags);
        ImGui::Text("In this Window, you can load OBJ files or textures with filepath.");
        ImGui::Text("You can still export all the scene to OBJ files");
        ImGui::NewLine();
        ImGui::Separator();
        ImGui::InputText("input file path", filepath, IM_ARRAYSIZE(filepath));
        if(ImGui::Button("Load OBJ file")){
            // TODO:Load OBJ file
        }
        ImGui::SameLine();
        if(ImGui::Button("Load Texture file")){
            // TODO:Load Texture file
        }
        ImGui::Separator();
        ImGui::Text("This will merge all the objects to a single group.");
        ImGui::NewLine();
        if(ImGui::Button("Export scene to OBJ")){
            // TODO::Export scene to OBJ
        }
        ImGui::End();
    }
    if(object_flags){
        ImGui::Begin("Object Window", &object_flags);
        ImGui::Text("In this Window, you can view all the objects in the scene including light.");
        ImGui::Text("You are free to set their scales, rotations and locations.");
        ImGui::Text("For normal object, you can set roughness and color.");
        ImGui::Text("For light, you can set intensity and color");
        ImGui::End();
    }
    if(NURBS_flags){
        ImGui::Begin("NURBS Window", &NURBS_flags);
        ImGui::Text("In this Window, you can build NURBS surface with mouse click.");
        ImGui::End();
    }
	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}