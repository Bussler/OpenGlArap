#pragma once

#include <glad\glad.h>
#include <GLFW\glfw3.h>
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>

namespace ShaderParser {

	struct ShaderProgramSource
	{
		std::string vertexSource;
		std::string fragmentSource;
	};

	//parse in shader source code and store in struct
	ShaderProgramSource parseShader(const std::string& file) {

		std::ifstream stream(file);

		enum class ShaderState
		{
			NONE, VERTEX, FRAGMENT
		};
		ShaderState curState = ShaderState::NONE;

		std::stringstream shaders[2]; //holds shaders to compile

		std::string line;
		while (std::getline(stream, line))//parse lines of input
		{

			if (line.find("#shader") != std::string::npos) { //we define a new shader
				if (line.find("vertex") != std::string::npos) { //vertex shader mode
					curState = ShaderState::VERTEX;
				}
				else if (line.find("fragment") != std::string::npos) { //fragment shader mode
					curState = ShaderState::FRAGMENT;
				}
			}
			else //add to vertex or frag shader
			{
				if (curState == ShaderState::VERTEX) {
					shaders[0] << line << "\n";
				}
				else if (curState == ShaderState::FRAGMENT) {
					shaders[1] << line << "\n";
				}

			}

		}

		return ShaderProgramSource{ shaders[0].str(), shaders[1].str() };
	}

	//link vertex and fragment shader together to shader program for use
	unsigned int createShader(const char *vSource, const char *fSource) {

		//vertex shader 1) create 2) bind source 3) compile
		unsigned int vertexShader;
		vertexShader = glCreateShader(GL_VERTEX_SHADER);
		glShaderSource(vertexShader, 1, &vSource, NULL);
		glCompileShader(vertexShader);

		int success; //error handling
		char infoLog[512];
		glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
		if (!success)
		{
			glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
			std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
		}

		//fragment shader
		unsigned int fragmentShader;
		fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
		glShaderSource(fragmentShader, 1, &fSource, NULL);
		glCompileShader(fragmentShader);

		glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
		if (!success)
		{
			glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
			std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;
		}

		//linking shaders to shader program
		unsigned int shaderProgram;
		shaderProgram = glCreateProgram();
		glAttachShader(shaderProgram, vertexShader);
		glAttachShader(shaderProgram, fragmentShader);
		glLinkProgram(shaderProgram);

		if (!success) {//error handling with linking
			glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
			std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
		}
		glDeleteShader(vertexShader);
		glDeleteShader(fragmentShader);

		//use linked program now with glUseProgram(shaderProgram); -> every rendering call after will use this shader
		return shaderProgram;
	}

}