#pragma once
#include <glad\glad.h>
#include <GLFW\glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm\gtc\type_ptr.hpp>
#include <string>
#include <vector>


struct Vertex {
	glm::vec3 Position;
	glm::vec3 Color;
	glm::vec3 Normal;
	glm::vec2 TexCoords;
};

//not supported for rendering yet
struct Texture {
	unsigned int id;
	std::string type; //diffuse, specular
};

//class to hold info about mesh and perform rendering
class Mesh
{
public:
	//data of Mesh
	std::vector<Vertex> vertices;
	std::vector<unsigned int> indices; //for indices drawing with EBO
	std::vector<Texture> textures; //TODO

	//constructor
	Mesh(std::vector<Vertex> vertices, std::vector<unsigned int> indices, std::vector<Texture> textures) {
		this->vertices = vertices;
		this->indices = indices;
		this->textures = textures;

		setupMesh();
	}


	void Draw(unsigned int shader) { //use shader to render mesh
		//TODO textures...

		//draw mesh
		glUseProgram(shader); //activate shader to draw on
		glBindVertexArray(VAO); //hold info how to read data in buffer
		glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0); //performs rendering

		glBindVertexArray(0);
	}

	void DrawModelViewProjection(unsigned int shader, glm::mat4 model, glm::mat4 view, glm::mat4 projection) { //use shader to render mesh, bind model, view, projection matrices to shader
		//TODO textures...

		//draw mesh
		glUseProgram(shader); //activate shader to draw on

		//bind model, view, projection
		int modelLoc = glGetUniformLocation(shader, "model");
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		int viewLoc = glGetUniformLocation(shader, "view");
		glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
		int projLoc = glGetUniformLocation(shader, "projection");
		glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

		glBindVertexArray(VAO); //hold info how to read data in buffer
		glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0); //performs rendering

		glBindVertexArray(0);
	}

	//update vertex data in VBO
	void UpdateMeshVertices() {
		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * vertices.size(), &vertices[0], GL_STATIC_DRAW); //copy vertex data into buffer for opengl to use
	}

private:
	//data/ buffers for rendering with OpenGl
	unsigned int VAO; //settings about reading vertex data from buffer and interpret it
	unsigned int VBO; //raw data about vertices (pos, color...)
	unsigned int EBO; //holds index data, referenced by VAO

	void setupMesh() {//init OpenGl buffers
		glGenVertexArrays(1, &VAO);
		glGenBuffers(1, &VBO);
		glGenBuffers(1, &EBO);

		glBindVertexArray(VAO);//store layout in VAO

		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * vertices.size(), &vertices[0], GL_STATIC_DRAW); //copy vertex data into buffer for opengl to use

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices[0], GL_STATIC_DRAW);

		//enable vertex attributes
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0); //vertex attribute: vertex pos = location 0 in vertex shader
		glEnableVertexAttribArray(0);

		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Color)); //color
		glEnableVertexAttribArray(1);

		glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Normal)); //normals
		glEnableVertexAttribArray(2);

		glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, TexCoords)); //tex coords
		glEnableVertexAttribArray(3);

		glBindVertexArray(0); //set back to default
	}

};