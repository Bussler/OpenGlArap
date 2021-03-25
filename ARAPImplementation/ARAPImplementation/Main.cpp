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

struct DragVertexData {
	float X; //x, y coords in screen space
	float Y;
	float NDCZ; //ndc depth for back projection
};

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);
void processInput(GLFWwindow *window);
void PickVertex(GLFWwindow* window, double xMouse, double yMouse);
void dragVertex(GLFWwindow* window, float xOffset, float yOffset);

Camera camera(glm::vec3(0, 0, 15), glm::vec3(0, 1, 0));
Model* ModelPointer;
std::vector<int> selectedConstraints;
std::vector<DragVertexData> selectedConstraintsData;

//model view prrojection matrices
glm::mat4 model = glm::mat4(1.0f);
glm::mat4 view = glm::mat4(1.0f);
glm::mat4 projection = glm::mat4(1.0f);

//time vals for movement
float deltaTime = 0.0f;	// time between current frame and last frame
float lastFrame = 0.0f;

//handling mouse input
bool firstMouse = true;
double lastXMPos;
double lastYMPos;

bool rotating = false;
bool dragging = false;


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

	//glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	glfwSetCursorPosCallback(window, mouse_callback); // register mouse callback funct
	glfwSetMouseButtonCallback(window, mouse_button_callback);

	//use glad to load opengl func pointer
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialize GLAD" << std::endl;
		return -1;
	}

	glEnable(GL_DEPTH_TEST); //activates depth test

	//parse and create shaders
	ShaderParser::ShaderProgramSource sSource = ShaderParser::parseShader("shaders/Color.shader"); //parse source code
	const char *vSource = sSource.vertexSource.c_str();
	const char *fSource = sSource.fragmentSource.c_str();
	unsigned int shaderProgramBasic = ShaderParser::createShader(vSource, fSource); //create vertex, fragment shaders and link together to program

	Model parsedModel("data/cactus.obj"); //model to render
	ModelPointer = &parsedModel;

	//use model view projection matrices to transform vertices from local to screen (NDC) space. NDC -> ViewPort is done automatically by opengl
	model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f)); //Caution: here we interchange y and z axis for this model!
	model = glm::translate(model, glm::vec3(0.0f, 0.0f, -4.0f));

	//view = camera.getViewMatrix();
	view = glm::translate(view, glm::vec3(0, 0, -13));

	projection = glm::perspective(glm::radians(45.0f), 800.0f / 600.0f, 0.1f, 100.0f);//perspective projection

	//Debugging
	//parsedModel.meshes[0].vertices[0].Color = glm::vec3(1.0f, 0.0f, 0.0f);	
	//parsedModel.meshes[0].UpdateMeshVertices();


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

		//for camera: update view matrix from camera
		//view = camera.getViewMatrix();

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


	if (rotating) { //rotateMesh
		model = glm::rotate(model, glm::radians(1.0f)*xoffset, glm::vec3(0.0f, 0.0f, 1.0f));
	}
	else {
		camera.processMouseCamera(xoffset, yoffset);
	}

	if (dragging) { //drag
		dragVertex(window, xoffset, yoffset);
	}

}

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
	if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS)
	{
		double xpos, ypos;
		//getting cursor position
		glfwGetCursorPos(window, &xpos, &ypos);
		std::cout << "Cursor Position at (" << xpos << " : " << ypos <<")" << std::endl;
		PickVertex(window, xpos, ypos);
	}

	if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) { //dragging
		dragging = true;
	}
	if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE) {
		dragging = false;
	}

	if (button == GLFW_MOUSE_BUTTON_MIDDLE && action == GLFW_PRESS) { //rotating mesh
		rotating = true;
	}
	if (button == GLFW_MOUSE_BUTTON_MIDDLE && action == GLFW_RELEASE) {
		rotating = false;
	}
}


void PickVertex(GLFWwindow* window, double xMouse, double yMouse) {

	for (int i = 0; i < ModelPointer->meshes[0].vertices.size(); i++) {
		glm::vec4 vertPos(ModelPointer->meshes[0].vertices[i].Position, 1);
		glm::vec4 pos = projection * view * model * vertPos; //ModelViewProjection
		pos.x /= pos.w; //Perspective division to NDC
		pos.y /= pos.w;
		pos.z /= pos.w;

		int width, height;
		glfwGetWindowSize(window, &width, &height);

		float X = (pos.x + 1.0f) * width * 0.5; //Get ViewPort coords
		float Y = (1.0f - pos.y) * height * 0.5;
		float Z = 0.1f + pos.z * (100.0f - 0.1f);

		float radiusX = 5.0f;
		float radiusY = 5.0f;

		if (abs(X - xMouse) < radiusX && abs(Y - yMouse) < radiusY) {//found click
			auto searchPos = std::find(selectedConstraints.begin(), selectedConstraints.end(), i);
			int index = std::distance(selectedConstraints.begin(), searchPos);

			if (searchPos != selectedConstraints.end()) { //constraint already selected
				ModelPointer->meshes[0].vertices[i].Color = glm::vec3(0.0f, 71.8f, 92.2f); //orig color
				selectedConstraints.erase(searchPos);
				selectedConstraintsData.erase(selectedConstraintsData.begin()+index);
			}
			else { //select
				ModelPointer->meshes[0].vertices[i].Color = glm::vec3(1.0f, 0.0f, 0.0f);
				selectedConstraints.push_back(i);
				selectedConstraintsData.push_back(DragVertexData{ X, Y, pos.z });
			}

			ModelPointer->meshes[0].UpdateMeshVertices();
			/*std::cout << "Vertex Pos orig: " << vertPos.x << " : " << vertPos.y << " : " << vertPos.z << std::endl;
			std::cout << "Vertex Pos NDC: " << pos.x << " : " << pos.y << " : " << pos.z << std::endl;
			std::cout << "Vertex Pos SCREEN: " << X << " : " << Y << " : " << Z << std::endl;*/
			
			//break; //get all vertices on the edge, not only the first one! -> vertex counts not for multiple faces!!!
		}

	}

}

void dragVertex(GLFWwindow* window, float xOffset, float yOffset) { //drag all safed constraints

	for (int i = 0; i < selectedConstraints.size();i++) { //loop through all selected constraints and apply the offset to them
		int vertexIndex = selectedConstraints.at(i);
		DragVertexData vData = selectedConstraintsData.at(i);

		vData.X += xOffset; //apply mouse input
		vData.Y -= yOffset;
		selectedConstraintsData.at(i) = vData;//saving

		//project back into model space to save into vertex data
		int width, height;
		glfwGetWindowSize(window, &width, &height);

		float ndcX = (vData.X *2) / width -1; //get ndc
		float ndcY = ((vData.Y * 2) / height -1) *(-1);


		glm::mat4 inverseModelViewProj = glm::inverse(projection * view * model); //inverse projection
		glm::vec4 reconstructedOrig = inverseModelViewProj * glm::vec4(ndcX, ndcY, selectedConstraintsData.at(i).NDCZ, 1);
		reconstructedOrig /= reconstructedOrig.w;

		//update the model
		ModelPointer->meshes[0].vertices[vertexIndex].Position = glm::vec3(reconstructedOrig.x, reconstructedOrig.y, reconstructedOrig.z);
		ModelPointer->meshes[0].UpdateMeshVertices();

	}

}

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	glViewport(0, 0, width, height);
}