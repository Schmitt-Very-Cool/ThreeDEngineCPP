#include <iostream>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <stb/stb_image.h>

#include <fstream>
#include <sstream>
#include <streambuf>
#include <string>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "Shader.h"

#include "../io/Keyboard.h"
#include "../io/Mouse.h"
#include "../io/Joystick.h"

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow* window);

float mixVal = 0.5f;

glm::mat4 transform = glm::mat4(1.0f);
Joystick mainJ(0);

int main() {
	int success;
	char infoLog[512];

	std::cout << "Hello World!" << std::endl;

	glfwInit();

	//version 3.3
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);

	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

	GLFWwindow* window = glfwCreateWindow(800, 600, "ThreeDEngine", NULL, NULL);
	if (window == NULL) { // window not created
		std::cout << "Could not create window." << std::endl;
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);

	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
		std::cout << "Failed to initialize GLAD" << std::endl;
		glfwTerminate();
		return -1;
	}

	glViewport(0, 0, 800, 600);

	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

	glfwSetKeyCallback(window, Keyboard::keyCallback);

	glfwSetCursorPosCallback(window, Mouse::cursorPosCallback);
	glfwSetMouseButtonCallback(window, Mouse::mouseButtonCallback);
	glfwSetScrollCallback(window, Mouse::mouseWheelCallback);

	// SHADERS===================================
	Shader shader("assets/vertex_core.glsl", "assets/fragment_core.glsl");

	// vertex array
	float vertices[] = {
		//positions				colors				texture coordinates
	    -0.5f, -0.5f, 0.0f,	    1.0f, 1.0f, 0.5f,   0.0f, 0.0f, //bottom left
	    -0.5f,  0.5f, 0.0f,	    0.5f, 1.0f, 0.75f,  0.0f, 1.0f, //top left
	     0.5f, -0.5f, 0.0f,	    0.6f, 1.0f, 0.2f,   1.0f, 0.0f, //bottom right
	     0.5f,  0.5f, 0.0f,	    1.0f, 0.2f, 1.0f,   1.0f, 1.0f  //top right
	};
	unsigned int indices[] = {
		0, 1, 2,	//first triangle
		3, 1, 2		//second triangle
	};

	// VAO, VBO, EBO
	unsigned int VAO, VBO, EBO;
	glGenBuffers(1, &VBO);
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &EBO);

	//bind VAO
	glBindVertexArray(VAO);

	//bind VBO
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	//put index array in EBO
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

	//set attribute pointers
	//position
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	//color
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);
	//texture coordinate attribute
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
	glEnableVertexAttribArray(2);

	// TEXTURES -------------------------------------------

	//generate texture
	unsigned int texture1, texture2;

	glGenTextures(1, &texture1);
	glBindTexture(GL_TEXTURE_2D, texture1); //bind to texture unit 0

	// image wrap (s, t, r) = (x, y, z)
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	// image filtering
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

	//load image
	int width, height, nChannels;
	stbi_set_flip_vertically_on_load(true);
	unsigned char* data = stbi_load("assets/image1.jpg", &width, &height, &nChannels, 0);

	if (data) {
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);
	}
	else {
		std::cout << "Failed to load texture" << std::endl;
	}

	stbi_image_free(data);

	glGenTextures(1, &texture2);
	glBindTexture(GL_TEXTURE_2D, texture2);

	// load image 2
	data = stbi_load("assets/image2.png", &width, &height, &nChannels, 0);

	if (data) {
		//RGBA because png
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);
	}

	stbi_image_free(data);

	shader.activate();
	shader.setInt("texture1", 0);
	shader.setInt("texture2", 1);

	//transformations
	/*glm::mat4 trans = glm::mat4(1.0f);
	trans = glm::rotate(trans, glm::radians(45.0f), glm::vec3(0.0f, 0.0f, 1.0f));
	shader.activate();
	shader.setMat4("transform", trans);*/

	mainJ.update();
	if (mainJ.isPresent()) {
		std::cout << mainJ.getName() << " is present." << std::endl;
	}
	else {
		std::cout << "Not present." << std::endl;
	}

	//MAIN LOOP --------------------------------------------------------------------------
	while (!glfwWindowShouldClose(window)) {
		//Process input
		processInput(window);

		//Render
		glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, texture1);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, texture2);

		//draw shapes
		glBindVertexArray(VAO);
		shader.activate();
		shader.setMat4("transform", transform);

		float timeValue = glfwGetTime();

		//trans = glm::rotate(trans, glm::radians(timeValue / 100), glm::vec3(0.0f, 0.5f, 0.5f));
		//shader.setMat4("transform", trans);

		shader.setFloat("mixVal", mixVal);

		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

		glBindVertexArray(0);

		//Send new frame to window
		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	glDeleteVertexArrays(1, &VBO);
	glDeleteBuffers(1, &VAO);
	glDeleteBuffers(1, &EBO);

	glfwTerminate();
	return 0;
}


void framebuffer_size_callback(GLFWwindow* window, int width, int height){
	glViewport(0, 0, width, height);
}

void processInput(GLFWwindow* window) {
	if (Keyboard::key(GLFW_KEY_ESCAPE) || mainJ.buttonState(GLFW_JOYSTICK_BTN_RIGHT)) {
		glfwSetWindowShouldClose(window, true);
	}

	//change mix value
	if (Keyboard::key(GLFW_KEY_UP)) {
		mixVal += 0.001f;
		if (mixVal > 1) {
			mixVal = 1.0f;
		}
	}
	if (Keyboard::key(GLFW_KEY_DOWN)) {
		mixVal -= 0.001f;
		if (mixVal < 0) {
			mixVal = 0.0f;
		}
	}

	if (Keyboard::key(GLFW_KEY_W)) {
		transform = glm::translate(transform, glm::vec3(0.0f, 0.01f, 0.0f));
	}
	if (Keyboard::key(GLFW_KEY_S)) {
		transform = glm::translate(transform, glm::vec3(0.0f, -0.01f, 0.0f));
	}
	if (Keyboard::key(GLFW_KEY_A)) {
		transform = glm::translate(transform, glm::vec3(-0.01f, 0.0f, 0.0f));
	}
	if (Keyboard::key(GLFW_KEY_D)) {
		transform = glm::translate(transform, glm::vec3(0.01f, 0.0f, 0.0f));
	}

	mainJ.update();

	float lx = mainJ.axesState(GLFW_JOYSTICK_AXES_LEFT_STICK_X);
	float ly = -mainJ.axesState(GLFW_JOYSTICK_AXES_LEFT_STICK_Y);

	if (std::abs(lx) > 0.5f) {
		transform = glm::translate(transform, glm::vec3(lx / 10, 0.0f, 0.0f));
	}
	if (std::abs(ly) > 0.5f) {
		transform = glm::translate(transform, glm::vec3(0.0f, ly / 10, 0.0f));
	}

	float rt = mainJ.isPresent() ? mainJ.axesState(GLFW_JOYSTICK_AXES_RIGHT_TRIGGER) / 2 + 0.5f : 0;
	if (rt > 0.05f) {
		transform = glm::scale(transform, glm::vec3(1 + rt / 10, 1 + rt / 10, 0.0f));
	}

	float lt = mainJ.isPresent() ? mainJ.axesState(GLFW_JOYSTICK_AXES_LEFT_TRIGGER) / 2 + 0.5f : 0;
	if (lt > 0.05f) {
		transform = glm::scale(transform, glm::vec3(1 - lt / 10, 1 - lt / 10, 0.0f));
	}
}