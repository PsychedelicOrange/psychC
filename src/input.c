#include "input.h"

static input* x = NULL;

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    // NOTE: Key events with GLFW_REPEAT actions are intended for text input
    switch(action){
	case GLFW_PRESS:
	    x->last_pressed[key] = glfwGetTime();
	    x->keys[key] = action;
	    break;
	case GLFW_RELEASE:
	    x->keys[key] = action;
	    break;
    }
}
void set_key_callback(GLFWwindow* window, input* i){
    x = i;
    glfwSetKeyCallback(window,key_callback);
}
