#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <fstream>
#include <sstream>

class Shader
{
public:
	GLuint ID;

	Shader(const char* vertShdPath, const char* fragShdPath)
	{
		/*Read files*/
		std::ifstream ifsVert(vertShdPath);
		std::ifstream ifsFrag(fragShdPath);

		if (!ifsVert)
		{
			std::cerr << "The vertex shader path is incorrect: " << vertShdPath << '\n';
			glfwTerminate();
		}
		if (!ifsFrag)
		{
			std::cerr << "The fragment shader path is incorrect: " << fragShdPath << '\n';
			glfwTerminate();
		}

		std::stringstream ssVert;
		std::stringstream ssFrag;
		ssVert << ifsVert.rdbuf();
		ssFrag << ifsFrag.rdbuf();

		std::string vertShdSrc = ssVert.str();
		std::string fragShdSrc = ssFrag.str();

		/*Now create shader program*/
		GLuint vertShad = glCreateShader(GL_VERTEX_SHADER);
		GLuint fragShad = glCreateShader(GL_FRAGMENT_SHADER);

		const char* vertSrc = vertShdSrc.c_str();
		const char* fragSrc = fragShdSrc.c_str();
		glShaderSource(vertShad, 1, &vertSrc, nullptr);
		glShaderSource(fragShad, 1, &fragSrc, nullptr);

		glCompileShader(vertShad);
		glCompileShader(fragShad);

		// check for shader compile errors
		int success;
		char infoLog[512];
		glGetShaderiv(vertShad, GL_COMPILE_STATUS, &success);
		if (!success)
		{
			glGetShaderInfoLog(vertShad, 512, nullptr, infoLog);
			std::cerr << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
		}

		glGetShaderiv(fragShad, GL_COMPILE_STATUS, &success);
		if (!success)
		{
			glGetShaderInfoLog(fragShad, 512, nullptr, infoLog);
			std::cerr << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;
		}

		this->ID = glCreateProgram();
		glAttachShader(this->ID, vertShad);
		glAttachShader(this->ID, fragShad);
		glLinkProgram(this->ID);

		glGetProgramiv(this->ID, GL_LINK_STATUS, &success);
		if (!success) {
			glGetProgramInfoLog(this->ID, 512, nullptr, infoLog);
			std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
		}

		glDeleteShader(vertShad);
		glDeleteShader(fragShad);
	}

	void use() const
	{
		glUseProgram(this->ID);
	}

	GLint getUniformLocation(const char* name) const
	{
		return glGetUniformLocation(this->ID, name);
	}

	void setUniform(const char* name, GLfloat value) const
	{
		glUniform1f(this->getUniformLocation(name), value);
	}

	void setUniform(const char* name, GLfloat v1, GLfloat v2) const
	{
		glUniform2f(this->getUniformLocation(name), v1, v2);
	}

	void setUniform(const char* name, GLfloat v1, GLfloat v2, GLfloat v3) const
	{
		glUniform3f(this->getUniformLocation(name), v1, v2, v3);
	}

	void setUniform(const char* name, GLfloat v1, GLfloat v2, GLfloat v3, GLfloat v4) const
	{
		glUniform4f(this->getUniformLocation(name), v1, v2, v3, v4);
	}

	void setUniform(const char* name, GLuint value) const
	{
		glUniform1ui(this->getUniformLocation(name), value);
	}

	void setUniform(const char* name, GLint value) const
	{
		glUniform1i(this->getUniformLocation(name), value);
	}

	void setUniform(const char* name, glm::mat4 const& matrix) const
	{
		glUniformMatrix4fv(this->getUniformLocation(name), 1, GL_FALSE, glm::value_ptr(matrix));
	}

	void setUniform(const char* name, glm::f32*	val) const
	{
		glUniformMatrix4fv(this->getUniformLocation(name), 1, GL_FALSE, val);
	}
	void setUniform(const char* name, const glm::f32* val) const
	{
		glUniformMatrix4fv(this->getUniformLocation(name), 1, GL_FALSE, val);
	}
};

