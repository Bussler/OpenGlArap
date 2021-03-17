#include <glad\glad.h>
#include <GLFW\glfw3.h>
#include <iostream>
#include <fstream>
#include "ShaderParser.h"
#include "Camera.h"
#include "MeshLoader.h"

//for transformations
#include <glm\glm.hpp>
#include <glm\gtc\matrix_transform.hpp>
#include <glm\gtc\type_ptr.hpp>


void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void processInput(GLFWwindow *window);

Camera camera(glm::vec3(0, 0, 15), glm::vec3(0, 1, 0));

//time vals for movement
float deltaTime = 0.0f;	// time between current frame and last frame
float lastFrame = 0.0f;

//handling mouse input
bool firstMouse = true;
double lastXMPos;
double lastYMPos;


int main() {
	
	//set init gl version: 3.3, core
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	//create window
	GLFWwindow* window = glfwCreateWindow(800, 600, "ARAP", NULL, NULL);
	if (window == NULL)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback); //resizing window resized the screen space: register window callback

	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	glfwSetCursorPosCallback(window, mouse_callback); // register mouse callback funct

	//use glad to load opengl func pointer
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialize GLAD" << std::endl;
		return -1;
	}

	glEnable(GL_DEPTH_TEST); //activates depth test

	//parse and create shaders
	ShaderParser::ShaderProgramSource sSource = ShaderParser::parseShader("shaders/Basic.shader"); //parse source code
	const char *vSource = sSource.vertexSource.c_str();
	const char *fSource = sSource.fragmentSource.c_str();
	unsigned int shaderProgramBasic = ShaderParser::createShader(vSource, fSource); //create vertex, fragment shaders and link together to program

	Model parsedModel("data/cactus.obj"); //model to render

	//render loop
	while (!glfwWindowShouldClose(window))
	{
		//per frame logic: get time vals
		float currentFrame = glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;

		//user input
		processInput(window);

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); //clear depth and color buffer before doing a new rendering pass
		glClearColor(0.2f, 0.3f, 0.3f, 1.0f); //state
		glClear(GL_COLOR_BUFFER_BIT); //state setting, clear buffer. Also available: GL_COLOR_BUFFER_BIT; GL_DEPTH_BUFFER_BIT; GL_STENCIL_BUFFER_BIT

		//use model view projection matrices to transform vertices from local to screen (NDC) space. NDC -> ViewPort is done automatically by opengl
		glm::mat4 model = glm::mat4(1.0f);
		model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
		model = glm::translate(model, glm::vec3(0.0f, 0.0f, -5.0f));

		glm::mat4 view = camera.getViewMatrix();

		glm::mat4 projection;
		projection = glm::perspective(glm::radians(45.0f), 800.0f / 600.0f, 0.1f, 100.0f);//perspective projection

		//rendering
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE); //activate better view of vertices of mesh
		parsedModel.DrawModelViewProjection(shaderProgramBasic, model, view, projection); //render mesh

		glfwSwapBuffers(window); //buffer swapping to counter rendering artifacts
		glfwPollEvents(); //processing callbacks 
	}

	glfwTerminate();
	return 0;
}

//handling of user input
void processInput(GLFWwindow *window)
{
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);

	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
		camera.processKeyBoardCamera(FORWARD, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		camera.processKeyBoardCamera(BACKWARD, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
		camera.processKeyBoardCamera(LEFT, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		camera.processKeyBoardCamera(RIGHT, deltaTime);
}

void mouse_callback(GLFWwindow* window, double xpos, double ypos) {
	if (firstMouse) {
		lastXMPos = xpos;
		lastYMPos = ypos;
		firstMouse = false;
	}
	//calc offset of mouse since last frame
	float xoffset = xpos - lastXMPos;
	float yoffset = lastYMPos - ypos; // reversed since y-coordinates range from bottom to top
	lastXMPos = xpos;
	lastYMPos = ypos;

	camera.processMouseCamera(xoffset, yoffset);

}

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	glViewport(0, 0, width, height);
}