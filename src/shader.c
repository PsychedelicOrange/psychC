#include "shader.h"
#include <glad/glad.h>
#include <stdio.h>
#include "constants.h"
#include "disk.h"

unsigned int create_shader(char* vertexPath, char* fragPath){
		char* vertexShaderCode =read_string_from_disk(vertexPath);
		char* fragmentShaderCode = read_string_from_disk(fragPath);
		unsigned int vertexShader = compile_shader(vertexShaderCode, GL_VERTEX_SHADER);
		unsigned int fragmentShader = compile_shader(fragmentShaderCode, GL_FRAGMENT_SHADER);
		return create_program(vertexShader,fragmentShader);
}

unsigned int compile_shader(const char * shaderCode, int shaderType){
	unsigned int shader = glCreateShader(shaderType);
	glShaderSource(shader, 1, &shaderCode, 0);
	glCompileShader(shader);
	// check for shader compile errors
	int success = 0;
	char infoLog[512];
	glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		glGetShaderInfoLog(shader, 512, 0, infoLog);
		printf("[FATAL] failed to compile shader: %s", infoLog);
		return -1;
	}
	return shader;
}

unsigned int create_program(unsigned int vertexShader, unsigned int fragmentShader){
    unsigned int shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);
    // check for linking errors
	int success = 0;
	char infoLog[512];
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(shaderProgram, 512, 0, infoLog);
		printf("[FATAL] failed to link shaders: %s",infoLog);
		return -1;
    }
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
	return shaderProgram;
}
void setUniformMat4(unsigned int shaderProgram,mat4s matrix, char* location){
	glUseProgram(shaderProgram);
	int loc = glGetUniformLocation(shaderProgram,location);
	glUniformMatrix4fv(loc,1,GL_FALSE,&matrix.col[0].raw[0]);	
}
