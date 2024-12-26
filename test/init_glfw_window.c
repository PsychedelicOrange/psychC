#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <../src/boiler.h>
#include <stdio.h>

int main(){
	init_glfw(4,3);
	GLFWwindow* window = create_glfw_window(800,600,"init_glfw_window_test\0");
	if(!window){
		fprintf(stderr,"Unable to create window");
	}
	glfwMakeContextCurrent(window);
	while (!glfwWindowShouldClose(window))
	{
		glfwSwapBuffers(window);
		glfwPollEvents();
	}

}
