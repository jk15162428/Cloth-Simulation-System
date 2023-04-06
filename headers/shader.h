#pragma once

#include <glad/glad.h>
#include <string>
#include <fstream>
#include <sstream>
#include <filesystem>
#include <iostream>

class Shader
{
public:
	unsigned int ID; // ID of this shader program

	// Initialization: read file and construct shader program
	Shader() {}
	Shader(const char* vertexFilePath, const char* fragmentFilePath)
	{
		/** Read file **/
		std::ifstream vertexShaderFile, fragmentShaderFile;

		// Allow the ifstream can throw exception
		vertexShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
		fragmentShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);

		std::string vertexCode, fragmentCode;
		try
		{
			std::cout << "Try to read sources from \"" << vertexFilePath << "\" and \""
					  << fragmentFilePath << "\"" << std::endl;
			vertexShaderFile.open(vertexFilePath);
			fragmentShaderFile.open(fragmentFilePath);
			std::stringstream vertexShaderStream, fragmentShaderStream;

			vertexShaderStream << vertexShaderFile.rdbuf();
			fragmentShaderStream << fragmentShaderFile.rdbuf();

			vertexShaderFile.close();
			fragmentShaderFile.close();

			vertexCode = vertexShaderStream.str();
			fragmentCode = fragmentShaderStream.str();
		}
		catch (std::ifstream::failure e)
		{
			std::cout << "ERROR::SHADER::FILE_NOT_SUCCESSFULLY_READ" << std::endl;
		}
		const char* vertexShaderCode = vertexCode.c_str();
		const char* fragmentShaderCode = fragmentCode.c_str();
		/** End of read file **/

		/** Compile shader **/
		unsigned int vertexShader, fragmentShader;
		vertexShader = glCreateShader(GL_VERTEX_SHADER);
		glShaderSource(vertexShader, 1, &vertexShaderCode, NULL);
		glCompileShader(vertexShader);
		checkCompileAndLinkErrors(vertexShader, "COMPILE");

		// Same for fragment shader
		fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
		glShaderSource(fragmentShader, 1, &fragmentShaderCode, NULL);
		glCompileShader(fragmentShader);
		checkCompileAndLinkErrors(fragmentShader, "COMPILE");

		// Shader program
		ID = glCreateProgram();
		glAttachShader(ID, vertexShader);
		glAttachShader(ID, fragmentShader);
		glLinkProgram(ID);
		checkCompileAndLinkErrors(ID, "LINK");

		std::cout << "Shader created successfully, shader program ID: " << ID << std::endl;
		/** End of compile shader **/

		// Clean shaders and use program.
		glDeleteShader(vertexShader);
		glDeleteShader(fragmentShader);
	}

	void use() { glUseProgram(ID); }
	// public utility functions
	void setBool(const std::string& name, bool value) const
	{
		glUniform1i(glGetUniformLocation(ID, name.c_str()), (int)value);
	}
	void setInt(const std::string& name, int value) const
	{
		glUniform1i(glGetUniformLocation(ID, name.c_str()), value);
	}
	void setFloat(const std::string& name, float value) const
	{
		glUniform1f(glGetUniformLocation(ID, name.c_str()), value);
	}
	void setVec2(const std::string& name, const glm::vec2& value) const
	{
		glUniform2fv(glGetUniformLocation(ID, name.c_str()), 1, &value[0]);
	}
	void setVec2(const std::string& name, float x, float y) const
	{
		glUniform2f(glGetUniformLocation(ID, name.c_str()), x, y);
	}
	void setVec3(const std::string& name, const glm::vec3& value) const
	{
		glUniform3fv(glGetUniformLocation(ID, name.c_str()), 1, &value[0]);
	}
	void setVec3(const std::string& name, float x, float y, float z) const
	{
		glUniform3f(glGetUniformLocation(ID, name.c_str()), x, y, z);
	}
	void setVec4(const std::string& name, const glm::vec4& value) const
	{
		glUniform4fv(glGetUniformLocation(ID, name.c_str()), 1, &value[0]);
	}
	void setVec4(const std::string& name, float x, float y, float z, float w) const
	{
		glUniform4f(glGetUniformLocation(ID, name.c_str()), x, y, z, w);
	}
	void setMat2(const std::string& name, const glm::mat2& mat) const
	{
		glUniformMatrix2fv(glGetUniformLocation(ID, name.c_str()), 1, GL_FALSE, &mat[0][0]);
	}
	void setMat3(const std::string& name, const glm::mat3& mat) const
	{
		glUniformMatrix3fv(glGetUniformLocation(ID, name.c_str()), 1, GL_FALSE, &mat[0][0]);
	}
	void setMat4(const std::string& name, const glm::mat4& mat) const
	{
		glUniformMatrix4fv(glGetUniformLocation(ID, name.c_str()), 1, GL_FALSE, &mat[0][0]);
	}
private:
	// utility function for checking shader compilation/linking errors.
	void checkCompileAndLinkErrors(unsigned int shader, std::string type)
	{
		int successFlag;
		char infoLog[1024];
		if (type != "LINK")
		{
			glGetShaderiv(shader, GL_COMPILE_STATUS, &successFlag);
			if (!successFlag)
			{
				glGetShaderInfoLog(shader, 1024, NULL, infoLog);
				std::cout << "ERROR::SHADER_COMPILATION_ERROR of type: " << type << "\n" << infoLog << "\n -- --------------------------------------------------- -- " << std::endl;
			}
		}
		else
		{
			glGetProgramiv(shader, GL_LINK_STATUS, &successFlag);
			if (!successFlag)
			{
				glGetProgramInfoLog(shader, 1024, NULL, infoLog);
				std::cout << "ERROR::PROGRAM_LINKING_ERROR of type: " << type << "\n" << infoLog << "\n -- --------------------------------------------------- -- " << std::endl;
			}
		}
	}
};
