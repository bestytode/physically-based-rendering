#include <iostream>
#include <stdexcept>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "camera.h"
#include "shader.h"
#include "geometry_renderers.h"
#include "model.h"
#include "timer.h"

#include "imgui/imgui.h"
#include "imgui/imgui_impl_glfw.h"
#include "imgui/imgui_impl_opengl3.h"

// Callback function declarations
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void ProcessInput(GLFWwindow* window);
unsigned int LoadTexture(const std::string& path);

// Scene settings
int SCR_WIDTH = 1920;  // Screen width
int SCR_HEIGHT = 1080; // Screen height

// Camera settings
Camera camera(0.0f, 0.0f, 5.0f);
float plane_near = 0.1f;
float plane_far = 100.0f;
float lastX = (float)SCR_WIDTH / 2.0f;
float lastY = (float)SCR_HEIGHT / 2.0f;
bool mouseButtonPressed = true; // Only move the camera when pressing left mouse
bool enableCameraMovement = true; // Interact through ImGui UI panal

// Timing
float deltaTime = 0.0f;
float lastFrame = 0.0f;

int main()
{
	Timer timer;
	timer.start();

	// glfw & glew configs
	// -------------------
	GLFWwindow* window = nullptr;
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

		// OpenGL global status settings here
		glEnable(GL_DEPTH_TEST);
		//glEnable(GL_CULL_FACE);
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

	timer.stop();

	// Imgui settings
    // --------------
	bool ImGUIFirstTime = true;
	double cursor_x, cursor_y;
	unsigned char pixel[4];
	while (!glfwWindowShouldClose(window)) {
		// Per-frame logic
		float currentFrame = (float)glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;

		// Render
		glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// Process input
		ProcessInput(window);

		// ImGui code here
        // ---------------
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
		ImGui::End();

		// The second UI panal
		if (ImGUIFirstTime) {
			ImGui::SetNextWindowSize(ImVec2(500, 250));
			ImGui::SetNextWindowPos(ImVec2(50, 350));
			ImGUIFirstTime = false;
		}
		ImGui::Begin("PBR");
		// debug pbr parameters here
		// -------------------------
		// TODO
		ImGui::End();

		// ImGui Rendering
		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

		// glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	// Optional: release all the resources of OpenGL (VAO, VBO, etc.)
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
unsigned int LoadTexture(const std::string& path)
{
	unsigned int textureID;
	glGenTextures(1, &textureID);

	int width, height, nrComponents;
	unsigned char* data = stbi_load(path.c_str(), &width, &height, &nrComponents, 0);
	if (data) {
		GLenum format = GL_RGBA;
		if (nrComponents == 1) format = GL_RED;
		else if (nrComponents == 3) format = GL_RGB;
		else if (nrComponents == 4) format = GL_RGBA;
		else {
			std::cerr << "Invalid texture format: Unsupported number of components!\n";
			return -1;
		}

		glBindTexture(GL_TEXTURE_2D, textureID);
		glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		stbi_image_free(data);
	}
	else {
		std::cerr << "Texture failed to load at path: " << path << std::endl;
		stbi_image_free(data);
		return -1;
	}

	return textureID;
}