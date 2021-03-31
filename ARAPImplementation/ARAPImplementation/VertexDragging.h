#pragma once
#include "MeshLoader.h"

namespace vertexDragging {

	//struct to hold data of marked vertices in order to project back from screen -> model space
	struct DragVertexData {
		float X; //x, y coords in screen space
		float Y;
		float NDCZ; //ndc depth for back projection
	};

	bool changedDragVertexData = false;
	bool changedConstraints = false;

	//data for our vertices
	Model* ModelPointer; //get static Data in main.cpp
	std::vector<int> selectedConstraints; //Movable
	std::vector<DragVertexData> selectedConstraintsData;

	glm::vec3 dynamicConstraintColor(1.0f, 0.0f, 0.0f);
	glm::vec3 staticConstraintColor(17.0f, 100.0f, 56.0f);
	glm::vec3 origColor(0.0f, 71.8f, 92.2f);

	void setModel(Model* model) {
		ModelPointer = model;
	}

	//select vertices in the model that we want to drag
	void pickVertex(GLFWwindow* window, double xMouse, double yMouse, glm::mat4 modelViewProjection) {

		for (int i = 0; i < ModelPointer->meshes[0].vertices.size(); i++) {
			glm::vec4 vertPos(ModelPointer->meshes[0].vertices[i].Position, 1);
			glm::vec4 pos = modelViewProjection * vertPos; //ModelViewProjection: projection * view * model * vertPos
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

				if (searchPos != selectedConstraints.end()) { //constraint already selected -> put static
					glm::vec3 curColor = ModelPointer->meshes[0].vertices[i].Color;
					if (curColor == dynamicConstraintColor) { //dynamic
						ModelPointer->meshes[0].vertices[i].Color = staticConstraintColor; //static color
					}
					else { //static 
						ModelPointer->meshes[0].vertices[i].Color = origColor; //orig color

						selectedConstraints.erase(searchPos);
						selectedConstraintsData.erase(selectedConstraintsData.begin() + index);
						changedConstraints = true;
					}
					
				}
				else { //select
					ModelPointer->meshes[0].vertices[i].Color = dynamicConstraintColor;
					selectedConstraints.push_back(i);
					selectedConstraintsData.push_back(DragVertexData{ X, Y, pos.z }); //TODO how to update x, y, z when rotating the screen?
					changedConstraints = true;
				}

				ModelPointer->meshes[0].UpdateMeshVertices();

				/*std::cout << "Vertex Pos orig: " << vertPos.x << " : " << vertPos.y << " : " << vertPos.z << std::endl;
				std::cout << "Vertex Pos NDC: " << pos.x << " : " << pos.y << " : " << pos.z << std::endl;
				std::cout << "Vertex Pos SCREEN: " << X << " : " << Y << " : " << Z << std::endl;*/

				//break; //get all vertices on the edge, not only the first one! -> vertex counts not for multiple faces!!!
			}

		}

	}

	//checks if view space has changed since last drag and if so recalculates screen Pos X Y and NDCZ
	void updateDragVertexData(GLFWwindow* window, glm::mat4 modelViewProjection) {

		if (!changedDragVertexData) { //nothing to do, data is still up to date
			return;
		}
		else //recalculate screen Pos X Y and NDCZ
		{
			for (int i = 0; i < selectedConstraints.size(); i++) {
				int vertexIndex = selectedConstraints.at(i);
				glm::vec4 vertPos(ModelPointer->meshes[0].vertices[vertexIndex].Position, 1);
				glm::vec4 pos = modelViewProjection * vertPos; //ModelViewProjection: projection * view * model * vertPos
				pos.x /= pos.w; //Perspective division to NDC
				pos.y /= pos.w;
				pos.z /= pos.w;

				int width, height;
				glfwGetWindowSize(window, &width, &height);

				float X = (pos.x + 1.0f) * width * 0.5; //Get ViewPort coords
				float Y = (1.0f - pos.y) * height * 0.5;
				
				selectedConstraintsData.at(i) = DragVertexData{ X, Y, pos.z };
			}
			changedDragVertexData = false;
		}
	}

	//apply mouse input to all picked vertices of the model
	void dragVertices(GLFWwindow* window, float xOffset, float yOffset, glm::mat4 modelViewProjection) { //drag all safed constraints

		updateDragVertexData(window, modelViewProjection); //check for changes in modelViewProjection matrix

		for (int i = 0; i < selectedConstraints.size();i++) { //loop through all selected constraints and apply the offset to them
			int vertexIndex = selectedConstraints.at(i);
			DragVertexData vData = selectedConstraintsData.at(i);

			if (ModelPointer->meshes[0].vertices[vertexIndex].Color == staticConstraintColor) //skip static constraints
				break;

			vData.X += xOffset; //apply mouse input
			vData.Y -= yOffset;
			selectedConstraintsData.at(i) = vData; //saving update

			//project back into model space to save into vertex data
			int width, height;
			glfwGetWindowSize(window, &width, &height);

			float ndcX = (vData.X * 2) / width - 1; //get ndc
			float ndcY = ((vData.Y * 2) / height - 1) *(-1);


			glm::mat4 inverseModelViewProj = glm::inverse(modelViewProjection); //inverse projection
			glm::vec4 reconstructedOrig = inverseModelViewProj * glm::vec4(ndcX, ndcY, selectedConstraintsData.at(i).NDCZ, 1);
			reconstructedOrig /= reconstructedOrig.w;

			//update the model
			ModelPointer->meshes[0].vertices[vertexIndex].Position = glm::vec3(reconstructedOrig.x, reconstructedOrig.y, reconstructedOrig.z);
			ModelPointer->meshes[0].UpdateMeshVertices();

		}

	}

}