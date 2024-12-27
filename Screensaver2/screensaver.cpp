#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>


#include "shader_m.h"
#include "shader_c.h"

#include <iostream>

#include <Windows.h>


void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void renderQuad();

#define FULLSCREEN
#define DO_3D_FRACTALS

// timing 
float deltaTime = 0.0f; // time between current frame and last frame
float lastFrame = 0.0f; // time of last frame

const char* compute_source_1 =
#include "3D_fractals.compute"
;

const char* compute_source_2 =
#include "2D_fractals.compute"
;

const char* fragment_source =
#include "screenQuad.fs"
;

const char* vertex_source =
#include "screenQuad.vs"
;

bool ScreensaverShouldExit = false;
bool tIncreasing = true;

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	if (action == GLFW_PRESS) {
		ScreensaverShouldExit = true;
	}
}

static void cursor_position_callback(GLFWwindow* window, double xpos, double ypos)
{
	static bool FirstDetection = true;
	if (!FirstDetection) {
		ScreensaverShouldExit = true;
	}
	if (FirstDetection) {
		FirstDetection = false;
	}
}

void start_screensaver()
{
	// glfw: initialize and configure
	// ------------------------------
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

	// glfw window creation
	// --------------------

#ifndef FULLSCREEN
	int window_width = 800, window_height = 600;


	GLFWwindow* window = glfwCreateWindow(window_width, window_height, "LearnOpenGL", NULL, NULL);
#else
	GLFWmonitor* monitor = glfwGetPrimaryMonitor();
	const GLFWvidmode* mode = glfwGetVideoMode(monitor);

	glfwWindowHint(GLFW_RED_BITS, mode->redBits);
	glfwWindowHint(GLFW_GREEN_BITS, mode->greenBits);
	glfwWindowHint(GLFW_BLUE_BITS, mode->blueBits);
	glfwWindowHint(GLFW_REFRESH_RATE, mode->refreshRate);

	int window_width = mode->width;
	int window_height = mode->height;

	GLFWwindow* window = glfwCreateWindow(window_width, window_height, "My Title", monitor, NULL);
#endif
	if (window == NULL)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		// TODO: return -1;
		return;
	}
	glfwMakeContextCurrent(window);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	glfwSwapInterval(0);

	// glad: load all OpenGL function pointers
	// ---------------------------------------
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialize GLAD" << std::endl;
		// TODO: return -1;
		return;
	}

	glfwSetKeyCallback(window, key_callback);
#ifdef FULLSCREEN
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	glfwSetCursorPosCallback(window, cursor_position_callback);
#endif

	// query limitations
	// -----------------
	int max_compute_work_group_count[3];
	int max_compute_work_group_size[3];
	int max_compute_work_group_invocations;

	for (int idx = 0; idx < 3; idx++) {
		glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT, idx, &max_compute_work_group_count[idx]);
		glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_SIZE, idx, &max_compute_work_group_size[idx]);
	}
	glGetIntegerv(GL_MAX_COMPUTE_WORK_GROUP_INVOCATIONS, &max_compute_work_group_invocations);

	std::cout << "OpenGL Limitations: " << std::endl;
	std::cout << "maximum number of work groups in X dimension " << max_compute_work_group_count[0] << std::endl;
	std::cout << "maximum number of work groups in Y dimension " << max_compute_work_group_count[1] << std::endl;
	std::cout << "maximum number of work groups in Z dimension " << max_compute_work_group_count[2] << std::endl;

	std::cout << "maximum size of a work group in X dimension " << max_compute_work_group_size[0] << std::endl;
	std::cout << "maximum size of a work group in Y dimension " << max_compute_work_group_size[1] << std::endl;
	std::cout << "maximum size of a work group in Z dimension " << max_compute_work_group_size[2] << std::endl;

	std::cout << "Number of invocations in a single local work group that may be dispatched to a compute shader " << max_compute_work_group_invocations << std::endl;

	// build and compile shaders
	// -------------------------
	Shader screenQuad(vertex_source, fragment_source);
	//ComputeShader computeShader("main.compute");

#ifdef DO_3D_FRACTALS
	ComputeShader computeShader(compute_source_1);
#else
#ifdef DO_2D_FRACTALS
	ComputeShader computeShader(compute_source_2);
#else
	ComputeShader computeShader("");
#endif
#endif

	screenQuad.use();
	screenQuad.setInt("tex", 0);

	// Create texture for opengl operation
	// -----------------------------------
	unsigned int texture;

	glGenTextures(1, &texture);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, window_width, window_height, 0, GL_RGBA, GL_FLOAT, NULL);

	glBindImageTexture(0, texture, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texture);

	// render loop
	// -----------

	glm::vec3 position = glm::vec3(0.0f, 0.0f, -1.5f);
	int fCounter = 0;

	float t = -10.0f;
	while (!glfwWindowShouldClose(window) && !ScreensaverShouldExit)
	{
		// Set frame time
		float currentFrame = glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;
		if (t >= 10.0f) {
			tIncreasing = false;
		}
		
		if (tIncreasing) {
			t += deltaTime;
		}
		else {
			t -= deltaTime;
		}
		if (fCounter > 10) {
			std::cout << "Frame Time: " << deltaTime * 1000 << "ms\n";
			std::cout << "FPS: " << 1 / deltaTime << std::endl;
			fCounter = 0;
		}
		else {
			fCounter++;
		}

		computeShader.use();
		computeShader.setFloat("t", t);
		computeShader.setVec3("pos", position);
		glDispatchCompute((unsigned int)window_width, (unsigned int)window_height, 1);

		// make sure writing to image has finished before read
		glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

		// render image to quad
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		screenQuad.use();

		renderQuad();

		// glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
		// -------------------------------------------------------------------------------
		glfwSwapBuffers(window);
		glfwPollEvents();

		if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
			position.z -= 0.1f;
		}
		if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
			position.z += 0.1f;
		}
	}

	// optional: de-allocate all resources once they've outlived their purpose:
	// ------------------------------------------------------------------------
	glDeleteTextures(1, &texture);
	glDeleteProgram(screenQuad.ID);
	glDeleteProgram(computeShader.ID);

	glfwTerminate();
	// TODO: return EXIT_SUCCESS;
	return;
}

// renderQuad() renders a 1x1 XY quad in NDC
// -----------------------------------------
unsigned int quadVAO = 0;
unsigned int quadVBO;
void renderQuad()
{
	if (quadVAO == 0)
	{
		float quadVertices[] = {
			// positions        // texture Coords
			-1.0f,  1.0f, 0.0f, 0.0f, 1.0f,
			-1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
			 1.0f,  1.0f, 0.0f, 1.0f, 1.0f,
			 1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
		};
		// setup plane VAO
		glGenVertexArrays(1, &quadVAO);
		glGenBuffers(1, &quadVBO);
		glBindVertexArray(quadVAO);
		glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
	}
	glBindVertexArray(quadVAO);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	glBindVertexArray(0);
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	// make sure the viewport matches the new window dimensions; note that width and 
	// height will be significantly larger than specified on retina displays.
	glViewport(0, 0, width, height);
}