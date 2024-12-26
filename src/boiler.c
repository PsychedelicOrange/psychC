#include "boiler.h"
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "log.h"

void init_glfw(int gl_major, int gl_minor){
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, gl_major);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, gl_minor);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

}

void* create_glfw_window(int SCR_WIDTH, int SCR_HEIGHT, char* null_terminated_name){
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT,null_terminated_name, NULL, NULL);
    if (window == NULL)
    {
	loge("Unable to create glfw window");
	glfwTerminate();
    }
    glfwMakeContextCurrent(window);
    return window;
}

void init_glad(){
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
	loge("failed to initialize glad");
    }
    const GLubyte* vendor = glGetString(GL_VENDOR); // Returns the vendor
    const GLubyte* renderer = glGetString(GL_RENDERER); // Returns a hint to the model
    logi("GPU information");
    logi("Vendor: %s",vendor);
    logi("Renderer: %s\n",renderer);
}
