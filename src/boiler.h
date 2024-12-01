#ifndef BOILER_H
#define BOILER_H

void init_glfw(int gl_major,int gl_minor);
// returns GLFWwindow*
void* create_glfw_window(int SCR_WIDTH, int SCR_HEIGHT,char* null_terminated_name);
void init_glad();

#endif
