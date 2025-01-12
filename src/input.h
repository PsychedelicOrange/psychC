#ifndef INPUT_H_
#define INPUT_H_
#include <GLFW/glfw3.h>

typedef struct input {
    float last_pressed[GLFW_KEY_LAST];
    int keys[GLFW_KEY_LAST];
    float delta_time;
}input;

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
void set_key_callback(GLFWwindow* window, input* i);
#endif
