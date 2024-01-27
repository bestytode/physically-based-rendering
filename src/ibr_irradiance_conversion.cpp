// Introduction: PBR rendering with texture, with IBL for ambient lighting, we focus on 
// IBL diffuse lighting in this ibr_irradiance_conversion.cpp
// 
// Already #define GLEW_STATIC in preproccesor to specify static linking for glew 
// Dependencies: glfw, glew, glm, Assimp, ImGui, stb_image.h
// Using OpenGL 3.3 core version
// environment: Debug or Release with x64 with Visual Studio 2022
// 
// Author: Yu
// Date 2023/10/24

#include <iostream>
#include <stdexcept>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "camera.h"
#include "geometry_renderers.h"
#include "model.h"
#include "shader.h"
#include "timer.h"

#include "imgui/imgui.h"
#include "imgui/imgui_impl_glfw.h"
#include "imgui/imgui_impl_opengl3.h"
#include "scene_manager.h"

// Callback function declarations
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void ProcessInput(GLFWwindow* window);
unsigned int LoadTexture(const std::string& path, bool isHDR = false);
void CheckFramebufferStatus(unsigned int fbo, const std::string& framebufferName);

// Scene settings
int SCR_WIDTH = 1920;  // Screen width
int SCR_HEIGHT = 1080; // Screen height

// Camera settings
Camera camera(0.0f, 0.0f, 15.0f); // By default we set plane_near to 0.1f and plane_far to 100.0f
float lastX = (float)SCR_WIDTH / 2.0f;
float lastY = (float)SCR_HEIGHT / 2.0f;
bool mouseButtonPressed = true; // Move the camera only when pressing left mouse
bool enableCameraMovement = true; // Lock or unlock camera movement with UI panal

// Timing
float deltaTime = 0.0f;
float lastFrameTimePoint = 0.0f;

int main()
{
	Timer timer; // Timer that calculates init operation time
	timer.start(); // Timer starts

	// glfw & glew configs
	// -------------------
	GLFWwindow* window = nullptr; // GLFW window
	try {
		if (!glfwInit())
			throw std::runtime_error("failed to init glfw");
		window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "hnzz", nullptr, nullptr);
		if (!window)
			throw std::runtime_error("failed to create window");

		glfwMakeContextCurrent(window);
		glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
		glfwSetCursorPosCallback(window, mouse_callback);
		glfwSetScrollCallback(window, scroll_callback);

		if (glewInit() != GLEW_OK)
			throw std::runtime_error("failed to init glew");

		// OpenGL global settings
		// ----------------------
		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_LEQUAL); // set depth function to less than AND equal for skybox depth trick.
	}
	catch (const std::runtime_error& e) {
		std::cerr << e.what() << std::endl;
		return -1;
	}

	// ImGui Initialization
	// --------------------
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	io.FontGlobalScale = 2.0f;
	ImGui::StyleColorsDark();
	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init("#version 330 core");

	// Set VAO for geometry shape for later use
	// ----------------------------------------
	yzh::Quad quad;
	yzh::Cube cube;
	yzh::Sphere sphere(64, 64);

	// Shader(s) build & compile
	// -------------------------
	Shader pbr_ibl_diffuse_textured("res/shaders/pbr_ibl.vs", "res/shaders/pbr_ibl_diffuse_textured.fs"); 
	Shader pbr_ibl_diffuse("res/shaders/pbr_ibl.vs", "res/shaders/pbr_ibl_diffuse.fs");
	Shader equirectangular_to_cubemap_shader("res/shaders/cubemap.vs", "res/shaders/equirectangular_to_cubemap.fs");
	Shader irradiance_shader("res/shaders/cubemap.vs", "res/shaders/irradiance_convolution.fs");
	Shader background_shader("res/shaders/background.vs", "res/shaders/background.fs");
	Shader debug_light_shader("res/shaders/debug_light.vs", "res/shaders/debug_light.fs");

	background_shader.Bind();
	background_shader.SetInt("environmentMap", 0);

	// load PBR material textures
    // --------------------------
	unsigned int albedo = LoadTexture("res/textures/pbr/rusted_iron/albedo.png");
	unsigned int normal = LoadTexture("res/textures/pbr/rusted_iron/normal.png");
	unsigned int metallic = LoadTexture("res/textures/pbr/rusted_iron/metallic.png");
	unsigned int roughness = LoadTexture("res/textures/pbr/rusted_iron/roughness.png");
	unsigned int ao = LoadTexture("res/textures/pbr/rusted_iron/ao.png");
	// Scaling factors (control them in UI panal)
	float metallicScale = 1.0f; // Scale factor for metallic
	float roughnessScale = 1.0f; // Scale factor for roughness
	glm::vec3 albedoScale(1.0f, 1.0f, 1.0f); // Scale factor for albedo

	// lighting infos
    // --------------
	std::vector<glm::vec3> lightPositions = {
		glm::vec3(-10.0f,  10.0f, 10.0f),
		glm::vec3(10.0f,  10.0f, 10.0f),
		glm::vec3(-10.0f, -10.0f, 10.0f),
		glm::vec3(10.0f, -10.0f, 10.0f),
	};
	std::vector<glm::vec3> lightColors = {
		glm::vec3(300.0f, 300.0f, 300.0f),
		glm::vec3(300.0f, 300.0f, 300.0f),
		glm::vec3(300.0f, 300.0f, 300.0f),
		glm::vec3(300.0f, 300.0f, 300.0f)
	};
	int nrRows = 7;
	int nrColumns = 7;
	float spacing = 5.0f;

	// Set up framebuffer for generating environment cube map
	// ------------------------------------------------------
	unsigned int captureFBO, captureRBO;
	glGenFramebuffers(1, &captureFBO);
	glGenRenderbuffers(1, &captureRBO);
	glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
	glBindRenderbuffer(GL_RENDERBUFFER, captureRBO);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, 512, 512); // 512 * 512 used for environmentCubemap
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, captureRBO);

	// pbr: load the HDR environment map
	// ---------------------------------
	stbi_set_flip_vertically_on_load(true);
	int width, height, nrComponents;
	float* data = stbi_loadf("res/textures/hdr/newport_loft.hdr", &width, &height, &nrComponents, 0);
	unsigned int hdrTexture;
	if (data) {
		glGenTextures(1, &hdrTexture);
		glBindTexture(GL_TEXTURE_2D, hdrTexture);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, width, height, 0, GL_RGB, GL_FLOAT, data); // note how we specify the texture's data value to be float

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		stbi_image_free(data);
	}
	else
		std::cout << "Failed to load HDR image." << std::endl;
	
	// Set up cubemap to render to and attach to framebuffer
	// -----------------------------------------------------
	unsigned int environmentCubemap;
	glGenTextures(1, &environmentCubemap);
	glBindTexture(GL_TEXTURE_CUBE_MAP, environmentCubemap); // Notice we use GL_TEXTURE_CUBE_MAP
	for (int i = 0; i < 6; i++) {
		// Notice we use GL_RGB16F, GL_RGB, GL_FLOAT, which are all the same as hdrTexture
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB16F, 512, 512, 0, GL_RGB, GL_FLOAT, nullptr);
	}
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	// Set up projection and view matrices for capturing data onto the 6 cubemap face directions
	glm::mat4 captureProjection = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 10.0f); // Notice we have to use 90.0f
	std::vector<glm::mat4> captureViews = {
		glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
		glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(-1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
		glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  1.0f,  0.0f), glm::vec3(0.0f,  0.0f,  1.0f)),
		glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f,  0.0f), glm::vec3(0.0f,  0.0f, -1.0f)),
		glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  0.0f,  1.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
		glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  0.0f, -1.0f), glm::vec3(0.0f, -1.0f,  0.0f))
	};

	// convert HDR equirectangular environment map to cubemap equivalent
	// -----------------------------------------------------------------
	equirectangular_to_cubemap_shader.Bind();
	equirectangular_to_cubemap_shader.SetInt("equirectangularMap", 0);
	equirectangular_to_cubemap_shader.SetMat4("projection", captureProjection);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, hdrTexture);

	glViewport(0, 0, 512, 512);
	glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
	for (int i = 0; i < 6; i++) {
		equirectangular_to_cubemap_shader.SetMat4("view", captureViews[i]);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, environmentCubemap, 0);

#ifdef _DEBUG
		CheckFramebufferStatus(captureFBO, "captureFBO");
#endif
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		cube.Render(); // renders 1 * 1
	}
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	// Generating an irradiance cubemap, and re-scale capture FBO to irradiance scale
	// ------------------------------------------------------------------------------
	unsigned int irradianceMap;
	glGenTextures(1, &irradianceMap);
	glBindTexture(GL_TEXTURE_CUBE_MAP, irradianceMap);
	for (int i = 0; i < 6; ++i) {
		// Notice we change the size from 512 * 512 to 32 * 32
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB16F, 32, 32, 0, GL_RGB, GL_FLOAT, nullptr);
	}
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
	glBindRenderbuffer(GL_RENDERBUFFER, captureRBO);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, 32, 32); // Change the size of renderbuffer accordingly

	// pbr: solve diffuse integral by convolution to create an irradiance (cube)map.
	// -----------------------------------------------------------------------------
	irradiance_shader.Bind();
	irradiance_shader.SetInt("environmentMap", 0);
	irradiance_shader.SetMat4("projection", captureProjection);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_CUBE_MAP, environmentCubemap);

	glViewport(0, 0, 32, 32); // Change the size of viewport accordingly
	glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
	for (int i = 0; i < 6; ++i) {
		irradiance_shader.SetMat4("view", captureViews[i]);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, irradianceMap, 0);

#ifdef _DEBUG
		CheckFramebufferStatus(captureFBO, "captureFBO");
#endif
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		cube.Render(); // renders 1 * 1
	}
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	
	// config the viewport to the original framebuffer's screen dimensions before rendering
	int scrWidth, scrHeight;
	glfwGetFramebufferSize(window, &scrWidth, &scrHeight);
	glViewport(0, 0, scrWidth, scrHeight);
	
	// Imgui settings
	// --------------
	bool ImGUIFirstTime = true;
	double cursor_x, cursor_y;
	unsigned char pixel[4];

	timer.stop(); // Timer stops

	// Render loop
	// -----------
	while (!glfwWindowShouldClose(window)) {
		// Per-frame logic
		float currentFrameTimePoint = (float)glfwGetTime();
		deltaTime = currentFrameTimePoint - lastFrameTimePoint;
		lastFrameTimePoint = currentFrameTimePoint;

		// Process input
		ProcessInput(window);

		// Render
		glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// render scene, supplying the convoluted irradiance map to the final shader.
        // ------------------------------------------------------------------------------------------
		glm::mat4 projection = glm::perspective(glm::radians(camera.fov), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
		glm::mat4 view = camera.GetViewMatrix();
		glm::mat4 model = glm::mat4(1.0f);
		pbr_ibl_diffuse_textured.Bind();
		
		// texture units uniforms
		pbr_ibl_diffuse_textured.SetInt("albedoMap", 0);
		pbr_ibl_diffuse_textured.SetInt("normalMap", 1);
		pbr_ibl_diffuse_textured.SetInt("metallicMap", 2);
		pbr_ibl_diffuse_textured.SetInt("roughnessMap", 3);
		pbr_ibl_diffuse_textured.SetInt("aoMap", 4);
		pbr_ibl_diffuse_textured.SetInt("irradianceMap", 5);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, albedo);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, normal);
		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_2D, metallic);
		glActiveTexture(GL_TEXTURE3);
		glBindTexture(GL_TEXTURE_2D, roughness);
		glActiveTexture(GL_TEXTURE4);
		glBindTexture(GL_TEXTURE_2D, ao);
		glActiveTexture(GL_TEXTURE5);
		glBindTexture(GL_TEXTURE_CUBE_MAP, irradianceMap);

		// Lighting uniforms
		for (int i = 0; i < lightPositions.size(); ++i) {
			std::string posName = "lightPositions[" + std::to_string(i) + "]";
			std::string colName = "lightColors[" + std::to_string(i) + "]";
			pbr_ibl_diffuse_textured.SetVec3(posName, lightPositions[i]);
			pbr_ibl_diffuse_textured.SetVec3(colName, lightColors[i]);
		}
		pbr_ibl_diffuse_textured.SetMat4("projection", projection);
		pbr_ibl_diffuse_textured.SetMat4("view", view);
		pbr_ibl_diffuse_textured.SetVec3("viewPos", camera.position);

		// pbr scaling factors
		pbr_ibl_diffuse_textured.SetFloat("roughnessScale", roughnessScale);
		pbr_ibl_diffuse_textured.SetFloat("metallicScale", metallicScale);
		pbr_ibl_diffuse_textured.SetVec3("albedoScale", albedoScale);

		for (int row = 0; row < nrRows; row++) {
			for (int col = 0; col < nrColumns; col++) {
				// Clamp the roughness to 0.05 - 1.0 as perfectly smooth surfaces (roughness of 0.0) tend to look a bit on direct lighting.
				model = glm::mat4(1.0f);
				model = glm::translate(model, glm::vec3((col - (nrColumns / 2)) * spacing, (row - (nrRows / 2)) * spacing, 0.0f));
				model = glm::scale(model, glm::vec3(0.5f));

				pbr_ibl_diffuse_textured.SetMat4("model", model);
				pbr_ibl_diffuse_textured.SetMat3("normalMatrix", glm::transpose(glm::inverse(glm::mat3(model))));
				//sphere.Render();
			}
		}

		// Render pbr sphere without texture, changing metallic and roughness by row and col
		// ---------------------------------------------------------------------------------
		pbr_ibl_diffuse.Bind();
		pbr_ibl_diffuse.SetMat4("projection", projection);
		pbr_ibl_diffuse.SetMat4("view", view);
		pbr_ibl_diffuse.SetVec3("viewPos", camera.position);
		pbr_ibl_diffuse.SetVec3("albedo", 0.5f, 0.0f, 0.0f);
		pbr_ibl_diffuse.SetFloat("ao", 1.0f);

		// lighting uniforms
		for (int i = 0; i < lightPositions.size(); ++i) {
			std::string posName = "lightPositions[" + std::to_string(i) + "]";
			std::string colName = "lightColors[" + std::to_string(i) + "]";
			pbr_ibl_diffuse.SetVec3(posName, lightPositions[i]);
			pbr_ibl_diffuse.SetVec3(colName, lightColors[i]);
		}

		pbr_ibl_diffuse.SetInt("irradianceMap", 0);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_CUBE_MAP, irradianceMap);

		model = glm::mat4(1.0f);
		for (int row = 0; row < nrRows; ++row) {
			pbr_ibl_diffuse.SetFloat("metallic", (float)row / (float)nrRows);
			for (int col = 0; col < nrColumns; ++col) {
				pbr_ibl_diffuse.SetFloat("roughness", glm::clamp((float)col / (float)nrColumns, 0.05f, 1.0f));
				model = glm::mat4(1.0f);
				model = glm::translate(model, glm::vec3((float)(col - (nrColumns / 2)) * spacing, (float)(row - (nrRows / 2)) * spacing, -2.0f));
				pbr_ibl_diffuse.SetMat4("model", model);
				pbr_ibl_diffuse.SetMat3("normalMatrix", glm::transpose(glm::inverse(glm::mat3(model))));
				sphere.Render();
			}
		}

		// render light source
		// -------------------
		debug_light_shader.Bind();
		debug_light_shader.SetMat4("projection", projection);
		debug_light_shader.SetMat4("view", view);

		for (int i = 0; i < lightPositions.size(); ++i) {
			model = glm::mat4(1.0f);
			model = glm::translate(model, lightPositions[i]);
			model = glm::scale(model, glm::vec3(0.5f));
			debug_light_shader.SetMat4("model", model);
			sphere.Render();
		}

		// render skybox (render as last to prevent overdraw)
		// notice that we explicitly set depth value to 1.0f
		// -------------------------------------------------
		background_shader.Bind();
		background_shader.SetMat4("view", view);
		background_shader.SetMat4("projection", projection);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_CUBE_MAP, environmentCubemap);
		//glBindTexture(GL_TEXTURE_CUBE_MAP, irradianceMap); // display irradiance map
		cube.Render();

		// ImGui code
		// ----------
		// ImGui new frame 
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		// The first UI panal
		if (ImGUIFirstTime) {
			ImGui::SetNextWindowSize(ImVec2(500, 250));
			ImGui::SetNextWindowPos(ImVec2(50, 50));
		}
		ImGui::Begin("hnzz");
		ImGui::Text("FPS: %.1f", ImGui::GetIO().Framerate);
		// Retrieve and display the cursor position and the RGBA color of the pixel under the cursor
		glfwGetCursorPos(window, &cursor_x, &cursor_y);
		glReadPixels((int)cursor_x, (int)(SCR_HEIGHT - cursor_y), 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, pixel);

		ImGui::Text("Cursor position: (%.2f, %.2f)", cursor_x, cursor_y);
		ImGui::Text("RGBA: (%d, %d, %d, %d)", pixel[0], pixel[1], pixel[2], pixel[3]);
		ImGui::Checkbox("Enable camera movement", &enableCameraMovement);

		float newSpeed = camera.GetMovementSpeed(); 
		if (ImGui::SliderFloat("Movement Speed", &newSpeed, 0.1f, 5.0f)) 
			camera.SetMovementSpeed(newSpeed); 
		
		float newSensitivity = camera.GetMouseSensitivity(); 
		if (ImGui::SliderFloat("Mouse Sensitivity", &newSensitivity, 0.01f, 1.0f)) 
			camera.SetMouseSensitivity(newSensitivity); 

		ImGui::End();

		// The second UI panal
		if (ImGUIFirstTime) {
			ImGui::SetNextWindowSize(ImVec2(500, 250));
			ImGui::SetNextWindowPos(ImVec2(50, 350));
			ImGUIFirstTime = false;
		}
		ImGui::Begin("PBR");

		ImGui::Text("Metallic Scale"); // Metallic Scale
		ImGui::SameLine();
		ImGui::SliderFloat("##Metallic", &metallicScale, 0.0f, 2.0f);

		ImGui::Text("Roughness Scale"); // Roughness Scale
		ImGui::SameLine();
		ImGui::SliderFloat("##Roughness", &roughnessScale, 0.0f, 2.0f);

		ImGui::Text("Albedo Scale"); // Albedo Scale
		ImGui::SameLine();
		ImGui::SliderFloat3("##Albedo", &albedoScale[0], 0.0f, 2.0f);

		ImGui::End();

		// ImGui Rendering
		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

		// glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	// Release all the resources of OpenGL (VAO, VBO, etc.)
	glfwTerminate();

	// ImGui Cleanup
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();

	return 0;
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	// make sure the viewport matches the new window dimensions; note that SCR_WIDTH and 
	// SCR_HEIGHT will be significantly larger than specified on retina displays.
	glViewport(0, 0, width, height);
	SCR_WIDTH = width;
	SCR_HEIGHT = height;
}

// glfw: whenever the mouse moves, this callback is called
void mouse_callback(GLFWwindow* window, double xposIn, double yposIn)
{
	if (!enableCameraMovement)
		return;

	// Check if the left mouse button is pressed
	if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS) {
		float xpos = static_cast<float>(xposIn);
		float ypos = static_cast<float>(yposIn);

		if (mouseButtonPressed) {
			lastX = xpos;
			lastY = ypos;
			mouseButtonPressed = false;
		}

		float xoffset = xpos - lastX;
		float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top

		lastX = xpos;
		lastY = ypos;

		camera.ProcessMouseMovement(xoffset, yoffset);
	}
	else
		mouseButtonPressed = true;
}

// glfw: whenever the mouse scroll wheel scrolls, this callback is called
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	camera.ProcessMouseScroll(static_cast<float>(yoffset));
}

// Process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
void ProcessInput(GLFWwindow* window)
{
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);

	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
		camera.ProcessKeyboard(FORWARD, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		camera.ProcessKeyboard(BACKWARD, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
		camera.ProcessKeyboard(LEFT, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		camera.ProcessKeyboard(RIGHT, deltaTime);
}

// Utility function for loading a 2D texture from file
// Loading HDR texture when isHDR is true
unsigned int LoadTexture(const std::string& path, bool isHDR) 
{
	unsigned int textureID;
	glGenTextures(1, &textureID);

	int width, height, nrComponents;
	void* data;

	// Using stbi_load or stbi_loadf for different texture type
	if (!isHDR) {
		data = stbi_load(path.c_str(), &width, &height, &nrComponents, 0);
	}
	else {
		stbi_set_flip_vertically_on_load(true);
		data = stbi_loadf(path.c_str(), &width, &height, &nrComponents, 0);
	}

	if (data) {
		GLenum format;
		GLenum dataType = (isHDR) ? GL_FLOAT : GL_UNSIGNED_BYTE; // Set GL_FLOAT or GL_UNSIGNED_BYTE for different texture data type
		GLint internalFormat; // Using 16-bit float format for HDR, otherwise internalFormat equals to format

		if (nrComponents == 1) format = internalFormat = GL_RED;
		else if (nrComponents == 3) format = internalFormat = GL_RGB;
		else if (nrComponents == 4) format = internalFormat = GL_RGBA;
		else std::cerr << "Invalid texture format: Unsupported number of components!\n";

		if (isHDR) internalFormat = GL_RGB16F; 
		
		glBindTexture(GL_TEXTURE_2D, textureID);
		glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, format, dataType, data);
		if (!isHDR) 
			glGenerateMipmap(GL_TEXTURE_2D);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		if (!isHDR) 
			stbi_image_free(data);
		else 
			stbi_image_free((float*)data);
	}
	else {
		std::cerr << "Texture failed to load at path: " << path << std::endl;
		stbi_image_free(data);
		return -1;
	}

	return textureID;
}

void CheckFramebufferStatus(unsigned int fbo, const std::string& framebufferName) 
{
	glBindFramebuffer(GL_FRAMEBUFFER, fbo);
	GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	if (status != GL_FRAMEBUFFER_COMPLETE) {
		std::cerr << "Framebuffer (" << framebufferName << ") with ID (" << fbo << ") is not complete! Error code: " << status << std::endl;
		// You can further translate 'status' into specific error strings if you want
	}
	else {
		std::cout << "Framebuffer (" << framebufferName << ") with ID (" << fbo << ") is complete." << std::endl;
	}
}