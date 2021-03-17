#pragma once
#include <glad\glad.h>
#include <glm\glm.hpp>
#include <glm\gtc\matrix_transform.hpp>

enum CameraMoveDir
{
	FORWARD, BACKWARD, LEFT, RIGHT
};

class Camera {

public:
	glm::vec3 cameraPos; //pos of camera in world space
	glm::vec3 cameraViewDir; //view dir of camera in world space
	glm::vec3 worldUp; //up vector in world space

	//euler angles for looking around
	float yaw;
	float pitch;

	float moveSpeed; //movespeed for handling camera movement
	float mouseSensitivity;

	Camera(glm::vec3 cameraPos, glm::vec3 WorldUp, float moveSpeed = 5.0f, float mouseSensitivity = 0.1f) : cameraViewDir(glm::vec3(0.0f, 0.0f, -1.0f)), yaw(-90.0f), pitch(0.0f) { //setup of values
		this->cameraPos = cameraPos;
		this->worldUp = WorldUp;
		this->moveSpeed = moveSpeed;
		this->mouseSensitivity = mouseSensitivity;
	}

	glm::mat4 getViewMatrix() { //returns view space matrix for this camera
		glm::mat4 view = glm::lookAt(cameraPos, cameraPos + cameraViewDir, worldUp); //lookat: creates view space matrix that looks intor cameraViewDir
		return view;
	}

	//update cameraPos according to key inputs
	void processKeyBoardCamera(CameraMoveDir dir, float deltaTime) {
		float cameraSpeed = moveSpeed * deltaTime;

		switch (dir)
		{
		case FORWARD:
			cameraPos += cameraSpeed * cameraViewDir;
			break;
		case BACKWARD:
			cameraPos -= cameraSpeed * cameraViewDir;
			break;
		case LEFT:
			cameraPos -= cameraSpeed * glm::normalize(glm::cross(cameraViewDir, worldUp));
			break;
		case RIGHT:
			cameraPos += cameraSpeed * glm::normalize(glm::cross(cameraViewDir, worldUp));
			break;
		default:
			break;
		}
	}

	//update viewDir according to mouse input
	void processMouseCamera(double xOffset, double yOffset) {
		xOffset *= mouseSensitivity;
		yOffset *= mouseSensitivity;

		yaw += xOffset;
		pitch += yOffset;

		//constraints on camera: omit weird movement and lookAt flips
		if (pitch > 89.0f)
			pitch = 89.0f;
		if (pitch < -89.0f)
			pitch = -89.0f;

		//calculate view direction from euler angles with sin, cos on hypertenuse = 1
		glm::vec3 direction;
		direction.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
		direction.y = sin(glm::radians(pitch));
		direction.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
		cameraViewDir = glm::normalize(direction);

	}

};