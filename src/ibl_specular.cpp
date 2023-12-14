// Introduction: 
// 
// 
// Already #define GLEW_STATIC in preproccesor to specify static linking for glew 
// Dependencies: glfw, glew, glm, Assimp, ImGui, stb_image.h
// Using OpenGL 3.3 core version
// environment: Debug or Release with x64 with Visual Studio 2022
// 
// Author: Yu
// Date:

#include <iostream>
#include <stdexcept>
#include <memory>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "camera.h"
#include "shader.h"
#include "geometry_renderers.h"
#include "model.h"
#include "timer.h"
#include "scene_manager.h"

#include "imgui/imgui.h"
#include "imgui/imgui_impl_glfw.h"
#include "imgui/imgui_impl_opengl3.h"

int main()
{
	Timer timer; // Timer that calculates init operation time
	timer.start(); // Timer starts

	int SCR_WIDTH = 1920;  // Screen width
	int SCR_HEIGHT = 1080; // Screen height

    // Camera and SceneManager configs
	// -------------------------------
    std::shared_ptr<Camera> camera = std::make_shared<Camera>(0.0f, 0.0f, 3.0f); // Settings plane_near to 0.1f and plane_far to 100.0f by deault
	SceneManager scene_manager(SCR_WIDTH, SCR_HEIGHT, "hnzz", camera); // Holding GLFWwindow* and Camera object instance with utility functions
	
	// OpenGL global configs
	// ---------------------
	scene_manager.Enable(GL_DEPTH_TEST);

	// ImGui global configs
	// --------------------
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	io.FontGlobalScale = 2.0f;
	ImGui::StyleColorsDark();
	ImGui_ImplGlfw_InitForOpenGL(scene_manager.GetWindow(), true);
	ImGui_ImplOpenGL3_Init("#version 330 core");

	// Basic geometry shape for later use
	// ----------------------------------
	yzh::Quad quad;
	yzh::Cube cube;
	yzh::Sphere sphere(64, 64);
	yzh::Cone cone(1.0f, 1.0f, 20);

	// shader configs
	Shader shader("res/shaders/debug_light.vs", "res/shaders/debug_light.fs");

	// Imgui parameters
    // ----------------
	bool ImGUIFirstTime = true;
	double cursor_x = 0, cursor_y = 0;
	unsigned char pixel[4]{};

	timer.stop(); // Timer stops

	// Render loop
	while (!glfwWindowShouldClose(scene_manager.GetWindow())) {
		scene_manager.UpdateDeltaTime();
		scene_manager.ProcessInput();

		// Render
		glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// test rendering cones
		shader.Bind();
		glm::mat4 projection = glm::perspective(glm::radians(camera->fov), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
		glm::mat4 view = camera->GetViewMatrix();
		glm::mat4 model = glm::mat4(1.0f);
		model = glm::scale(model, glm::vec3(0.5f));
		shader.SetMat4("projection", projection);
		shader.SetMat4("view", view);
		shader.SetMat4("model", model);
		shader.SetInt("use_green_color", 1);
		sphere.Render();

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
		ImGui::End();

		// The second UI panal
		if (ImGUIFirstTime) {
			ImGui::SetNextWindowSize(ImVec2(500, 250));
			ImGui::SetNextWindowPos(ImVec2(50, 350));
			ImGUIFirstTime = false;
		}
		ImGui::Begin("PBR");
		ImGui::End();

		// ImGui Rendering
		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

		// glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
		glfwSwapBuffers(scene_manager.GetWindow());
		glfwPollEvents();
	}

	// ImGui Cleanup
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();

	return 0;
}