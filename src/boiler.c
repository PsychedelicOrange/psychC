#include "boiler.h"
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <stdio.h>

void init_glfw(int gl_major, int gl_minor){
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, gl_major);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, gl_minor);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

}

void* create_glfw_window(int SCR_WIDTH, int SCR_HEIGHT){
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "psychspiration", NULL, NULL);
    if (window == NULL)
    {
		printf("\n[FATAL] Unable to create glfw window");
        glfwTerminate();
    }
    glfwMakeContextCurrent(window);
	return window;
}

void init_glad(){
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
		printf("\n[FATAL] failed to initialize glad");
    }
	const GLubyte* vendor = glGetString(GL_VENDOR); // Returns the vendor
	const GLubyte* renderer = glGetString(GL_RENDERER); // Returns a hint to the model
	printf("\n[INFO] GPU information");
	printf("\n[INFO] Vendor: %s",vendor);
	printf("\n[INFO] Renderer: %s\n",renderer);
}
