#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <cglm/cglm.h>
#include <cglm/struct.h>
#include <stddef.h>
#define CGLTF_IMPLEMENTATION
#include "actor.h"
#include "boiler.h"
#include "shader.h"
#include "camera.h"
#include "log.h"
// --- -- -- - -- - - Defines -- -- -- - -- -- -- -- -- -- 
const vec3s up = {{0,1,0}};
float lastX,lastY;
camera cam;

// -- -- -- -- -- -- Function declare -- -- -- -- -- --- --
void framebuffer_size_callback(GLFWwindow *window, int width, int height);
static void cursor_position_callback(GLFWwindow* window, double xpos, double ypos);

// -- -- -- -- -- -- debug print math functions -- -- -- -- - -
//

GLuint setup_screen_quad(void){
    // Vertex data for the rectangle (two triangles forming a quad)
    float vertices[] = {
	// Positions (X, Y)
	-1.0f,  1.0f,  // Top-left
	-1.0f, -1.0f,  // Bottom-left
	1.0f, -1.0f,  // Bottom-right

	-1.0f,  1.0f,  // Top-left
	1.0f, -1.0f,  // Bottom-right
	1.0f,  1.0f   // Top-right
    };
    unsigned int VAO, VBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    // Position attribute
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    return VAO;
}

int main(int argc, char *argv[])
{

    init_glfw(4,3);
    GLFWwindow* window = (GLFWwindow*)create_glfw_window(800,600,"psychspiration\0");
    lastX = 800.0f/2;
    lastY = 600.0f/2;
    // callbacks and settings on window
    if (glfwRawMouseMotionSupported())
	glfwSetInputMode(window, GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);
    glfwSetCursorPosCallback(window, cursor_position_callback);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    cam = init_camera();

    init_glad();

    unsigned int environmentShader = create_shader("/home/parth/code/psychC/shaders/env.vs","/home/parth/code/psychC/shaders/env.fs");
    unsigned int actorShader = create_shader("/home/parth/code/psychC/shaders/actor.vs","/home/parth/code/psychC/shaders/actor.fs");
    logd("Shaders Loaded.");

    {
		mat4s projection = glms_perspective(glm_rad(45.0f), (float)800 / (float)600, 0.1f, 10000.0f);
		mat4s cube_transform = GLMS_MAT4_IDENTITY_INIT;
		setUniformMat4(actorShader,projection,"projection");
		setUniformMat4(actorShader,cube_transform,"model");
    }

    // load primitives for actors into mem
    model_actor model = load_model_actor(argv[1]);
    drawable_model dmodel = upload_model_actor(&model);

	// uncomment this call to draw in wireframe polygons.
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    // render loop
    // -----------
    while (!glfwWindowShouldClose(window))
    {
	// input
	// -----
	float speed = 1;
	if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
	{
	    dmodel.animations[0].started = glfwGetTime();
		logd("Started animation.");
	}
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
	{
	    glfwSetWindowShouldClose(window, 1);
	    logi("Exiting...\n");
	}
	if( GLFW_PRESS == glfwGetKey(window,GLFW_KEY_LEFT_SHIFT)){
	    speed = 0.1;
	}
	if( GLFW_PRESS == glfwGetKey(window, GLFW_KEY_E)){
	    cam.position.y += speed;
	}
	if( GLFW_PRESS == glfwGetKey(window, GLFW_KEY_Q)){
	    cam.position.y -= speed;
	}
	if( GLFW_PRESS == glfwGetKey(window, GLFW_KEY_W)){
	    cam.position.z -= speed;
	}
	if( GLFW_PRESS == glfwGetKey(window, GLFW_KEY_A)){
	    cam.position.x -= speed;
	}
	if( GLFW_PRESS == glfwGetKey(window, GLFW_KEY_S)){
	    cam.position.z += speed;
	}
	if( GLFW_PRESS == glfwGetKey(window, GLFW_KEY_D)){
	    cam.position.x += speed;
	}
	update_first_person_camera(&cam);

	// render
	// ------
	glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);

	// draw actors
	// set global shader variables
	glm_look(cam.position.raw,cam.front.raw,cam.up.raw,cam.lookAt.raw);
	setUniformMat4(actorShader,cam.lookAt,"view");

	draw_model(&dmodel,actorShader);
	//draw_single_primitive_env(actorShader,d);
	// glBindVertexArray(0); // no need to unbind it every time 

	// glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
	// -------------------------------------------------------------------------------
	glfwSwapBuffers(window);
	glfwPollEvents();
    }

    // optional: de-allocate all resources once they've outlived their purpose:
    glDeleteProgram(environmentShader);

    // glfw: terminate, clearing all previously allocated GLFW resources.
    // ------------------------------------------------------------------
    glfwTerminate();
    return 0;
}


// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow *window, int width, int height)
{
    // make sure the viewport matches the new window dimensions; note that width and 
    // height will be significantly larger than specified on retina displays.
    glViewport(0, 0, width, height);
}

static void cursor_position_callback(GLFWwindow* window, double xpos, double ypos)
{
    float xoffset = xpos - lastX;
    float yoffset = ypos - lastY ; // reversed since y-coordinates go from bottom to top
    lastX = xpos;
    lastY = ypos;

    xoffset *=  0.1;
    yoffset *=  0.1;

    update_camera_mouse_callback(&cam,xoffset,yoffset);
}
