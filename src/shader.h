#ifndef SHADER_H
#define SHADER_H

#include <cglm/cglm.h>
#include <cglm/struct.h>

unsigned int create_shader(char* vertexPath, char* fragPath);
unsigned int compile_shader(const char* source, int shaderType);
unsigned int create_program(unsigned int vertexShader,unsigned int fragmentShader);
void setUniformFloat4(unsigned int shaderProgram,float* floatArray, char* location);
void setUniformFloat(unsigned int shaderProgram,float number, char* location);
void setUniformInt(unsigned int shaderProgram,int integer, char* location);
void setUniformMat4(unsigned int shaderProgram,mat4s matrix, char* location);
void set_shader_texture_units(unsigned int shaderProgram);
#endif

