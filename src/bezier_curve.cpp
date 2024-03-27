#ifndef GLEW_STATIC
#define GLEW_STATIC
#endif 
#include <iostream>
#include <memory>
#include <random>
#include <stdexcept>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "camera.h"
#include "geometry_renderers.h"
#include "model.h"
#include "scene_manager.h"
#include "shader.h"
#include "timer.h"

#include "imgui/imgui.h"
#include "imgui/imgui_impl_glfw.h"
#include "imgui/imgui_impl_opengl3.h"


glm::vec3 calculateBezierPoint(float t, const std::vector<glm::vec3>& controlPoints) 
{
	float u = 1 - t;
	float tt = t * t;
	float uu = u * u;
	float uuu = uu * u;
	float ttt = tt * t;

	glm::vec3 p = uuu * controlPoints[0]; // first term
	p += 3 * uu * t * controlPoints[1]; // second term
	p += 3 * u * tt * controlPoints[2]; // third term
	p += ttt * controlPoints[3]; // fourth term

	return p;
}

int main()
{
	Timer timer; // Timer that calculates init operation time
	timer.start(); // Timer starts

	const int SCR_WIDTH = 1920;  // Screen width
	const int SCR_HEIGHT = 1080; // Screen height

	// shared pointer holding camera object. plane_near to 0.1f and plane_far to 100.0f by deault.
	std::shared_ptr<Camera> camera = std::make_shared<Camera>(0.0f, 0.0f, 3.0f);

	// Global scene manager holding window and camera object instance with utility functions.
	// Notice: glfw and glew init in SceneManager constructor.
	// -------------------------------------------------------
	SceneManager scene_manager(SCR_WIDTH, SCR_HEIGHT, "hnzz", camera);

	// OpenGL global configs
	// ---------------------
	scene_manager.Enable(GL_DEPTH_TEST);
	scene_manager.Enable(GL_MULTISAMPLE);
	glfwWindowHint(GLFW_SAMPLES, 4);
	scene_manager.Enable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	std::vector<glm::vec3> controlPoints(4); // Fixed array of 4 glm::vec3 for control points
	std::random_device rd;
	std::mt19937 gen(rd());
	std::uniform_real_distribution<> dis(-5.0, 5.0);

	for (auto& point : controlPoints) {
		point.x = dis(gen);
		point.y = dis(gen);
		point.z = 0.0f; // Fixed at 0.0
	}
	std::vector<glm::vec3> bezierCurvePoints;
	for (float t = 0; t <= 1.0f; t += 0.01f) { // Adjust step size as needed
		bezierCurvePoints.push_back(calculateBezierPoint(t, controlPoints));
	}

	GLuint VBO, VAO;
	glGenBuffers(1, &VBO);
	glGenVertexArrays(1, &VAO);

	// Bind the Vertex Array Object first, then bind and set vertex buffer(s), and then configure vertex attributes(s).
	glBindVertexArray(VAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, bezierCurvePoints.size() * sizeof(glm::vec3), &bezierCurvePoints[0], GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	unsigned int pointsVAO, pointsVBO;
	glGenBuffers(1, &pointsVBO);
	glGenVertexArrays(1, &pointsVAO);
	glBindVertexArray(pointsVAO);
	glBindBuffer(GL_ARRAY_BUFFER, pointsVBO);
	glBufferData(GL_ARRAY_BUFFER, controlPoints.size() * sizeof(glm::vec3), &controlPoints[0], GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	// ImGui global configs
	// --------------------
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	io.FontGlobalScale = 2.0f;
	ImGui::StyleColorsDark();
	ImGui_ImplGlfw_InitForOpenGL(scene_manager.GetWindow(), true);
	ImGui_ImplOpenGL3_Init("#version 330 core");

	// shader configs
	Shader shader("res/shaders/debug_light.vs", "res/shaders/debug_light.fs");

	// Imgui configs
	// ----------------
	float windowWidth = 480.0f, windowHeight = 1080.0f;
	float windowPosX = 0.0f, windowPosY = 0.0f;
	float fontSizeScale = 0.7f;

	timer.stop(); // Timer stops

	bool firstTimeOutputPosition = true;

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
		shader.SetInt("use_orange_color", 1);
		shader.SetInt("use_red_color", 0);
		glBindVertexArray(VAO);
		glDrawArrays(GL_LINE_STRIP, 0, bezierCurvePoints.size());
		glBindVertexArray(0);

		glPointSize(10.0f);
		shader.SetInt("use_orange_color", 0);
		shader.SetInt("use_red_color", 1);
		glBindVertexArray(pointsVAO); 
		glDrawArrays(GL_POINTS, 0, 4); 
		glBindVertexArray(0); 

		if (firstTimeOutputPosition) {
			for (size_t i = 0; i < controlPoints.size(); i++) {
				std::cout << "control points: " << controlPoints[i].x << " " << controlPoints[i].y << " " << controlPoints[i].z << std::endl;
			}
			firstTimeOutputPosition = false;
		}

		// ImGui code
		// ----------
		// ImGui new frame 
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		ImGui::SetNextWindowSize(ImVec2(windowWidth, windowHeight), ImGuiCond_Always);
		ImGui::SetNextWindowPos(ImVec2(windowPosX, windowPosY), ImGuiCond_Always);

		ImGuiStyle& style = ImGui::GetStyle();
		style.Colors[ImGuiCol_WindowBg].w = 0.7f;
		style.WindowRounding = 5.0f; // Adjust this value to increase or decrease the rounding
		style.FrameRounding = 5.0f; // For example, for buttons and other framed elements
		style.ChildRounding = 5.0f; // For child windows
		style.PopupRounding = 5.0f; // For pop-up windows

		// Adjust color for window background
		style.Colors[ImGuiCol_WindowBg] = ImVec4(0.10f, 0.10f, 0.10f, 0.7f); // Dark background with some transparency
		style.Colors[ImGuiCol_Header] = ImVec4(0.0f, 0.0f, 0.5f, 0.85f); // Blue color for headers
		style.Colors[ImGuiCol_HeaderHovered] = ImVec4(0.2f, 0.2f, 0.5f, 0.8f); // Slightly lighter blue when hovered
		style.Colors[ImGuiCol_HeaderActive] = ImVec4(0.3f, 0.3f, 0.5f, 0.9f); // Even lighter blue when active or clicked

		// Start the main window
		ImGui::Begin("Infos");
		ImGui::PushFont(ImGui::GetFont()); // Get default font
		ImGui::GetFont()->Scale = fontSizeScale;    // Scale the font size

		ImGui::Text("Rendering: TODO"); // TODO
		ImGui::Text("Profiling: TODO"); // TODO

		// Application info section
		if (ImGui::CollapsingHeader("Application Info", ImGuiTreeNodeFlags_DefaultOpen)) {
			ImGui::PushTextWrapPos(ImGui::GetCursorPos().x + windowWidth); // Wrap text at the panel width
			ImGui::Text("OpenGL Version: %s", glGetString(GL_VERSION));// Display OpenGL version
			ImGui::Text("Renderer: %s", glGetString(GL_RENDERER));// Display renderer information
			ImGui::Text("Vendor: %s", glGetString(GL_VENDOR));// Display vendor information
			ImGui::Text("GLSL Version: %s", glGetString(GL_SHADING_LANGUAGE_VERSION));// Display GLSL version
			ImGui::Text("Framerate: %.1f FPS", io.Framerate);// Display framerate information
			ImGui::PopTextWrapPos();
		}

		// About section
		if (ImGui::CollapsingHeader("About", ImGuiTreeNodeFlags_DefaultOpen)) {
			ImGui::Text("Author: Zhenhuan Yu");
			ImGui::Text("Email: yuzhenhuan99999@gmail.com");
			// Additional information about the engine or contact details can go here
		}

		// Revert to original font scale for other UI elements
		ImGui::PopFont();
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
}