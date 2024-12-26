#ifndef BOILER_H
#define BOILER_H

/*
 * Initialise glfw with opengl version and opengl profile and set platform specific window hints.
 * NOTE: OpenGL version will be set to 4.1 on Macs
 */
void init_glfw(int gl_major,int gl_minor);

/*
 * Create a new glfw window.
 * On failure, terminate glfw and return NULL.
 */
void* create_glfw_window(int SCR_WIDTH, int SCR_HEIGHT,char* null_terminated_name);

/* 
 * Initialise glad, and print gpu information.
 * On failure, exit the program.
 */
void init_glad();

#endif
