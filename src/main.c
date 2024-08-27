#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include "disk.h"
#include "constants.h"
#define CGLTF_IMPLEMENTATION
#include "cgltf.h"
// -- -- -- -- -- -- Structs -- -- -- -- -- -- -- -- -- --
struct vertice_pnt {
	float position[3];
	float normal[3];
	float texcoord[2];
};
struct vertice_pn {
	float position[3];
	float normal[3];
};
struct vertice_pntt {
	float position[3];
	float normal[3];
	float texcoord[2];
	float texcoord1[2];
};
// -- -- -- -- -- -- Function declare -- -- -- -- -- --- --
void crash_game(char* msg);

void init_glfw();
GLFWwindow* create_glfw_window();
void init_glad();

unsigned int compile_shader(const char* source, int shaderType);
unsigned int create_program(unsigned int vertexShader,unsigned int fragmentShader);

void framebuffer_size_callback(GLFWwindow *window, int width, int height);
void processInput(GLFWwindow *window);

// -- -- -- -- -- -- Contants -- -- -- -- -- --- --
// settings
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;


// -- -- -- -- -- -- GLFW Functions -- -- -- -- -- --- --
void init_glfw(){
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

}

GLFWwindow* create_glfw_window(){
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "psychspiration", NULL, NULL);
    if (window == NULL)
    {
		crash_game("Unable to create glfw window");
        glfwTerminate();
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	return window;
}

void init_glad(){
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
		crash_game("failed to initialize glad");
    }
}
// -- -- -- -- -- -- Shader functions -- -- -- -- -- --- --
//
unsigned int compile_shader(const char * shaderCode, int shaderType){
	unsigned int shader = glCreateShader(shaderType);
	glShaderSource(shader, 1, &shaderCode, NULL);
	glCompileShader(shader);
	// check for shader compile errors
	int success;
	char infoLog[512];
	glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		glGetShaderInfoLog(shader, 512, NULL, infoLog);
		printf("failed to compile shader: %s", infoLog);
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
	int success;
	char infoLog[512];
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
		printf("failed to link shaders: %s",infoLog);
		return -1;
    }
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
	return shaderProgram;
}

// -- -- -- -- -- -- Engine Helpers Functions -- -- -- -- -- --- --
void crash_game(char* msg){
	printf("Game crashed: %s",msg);
	exit(1);
}

// -- -- -- -- -- -- Cgltf parse helper functions -- -- -- -- - -
int cgltf_to_gl_type_indices[5] = {GL_INVALID_VALUE,GL_INVALID_VALUE,GL_INVALID_VALUE,GL_INVALID_VALUE,GL_UNSIGNED_SHORT,GL_UNSIGNED_INT};
typedef enum attribute_combo{
	attribute_combo_pn,
	attribute_combo_pnt,
	attribute_combo_pntt,
	attribute_combo_invalid
}attribute_combo;

attribute_combo get_attributes_present(cgltf_primitive p){
	int count = p.attributes_count;
	int pc=0,nc=0,tc=0,tanc=0;
	for(int i = 0;i < count;i++){
		switch(p.attributes[i].type){
			case cgltf_attribute_type_position:
				printf("\nposition attribute found");
				pc++;
				break;
			case cgltf_attribute_type_normal:
				printf("\nnormal attribute found");
				nc++;
				break;
			case cgltf_attribute_type_texcoord:
				printf("\ntexcoors attribute found");
				tc++;
				break;
			case cgltf_attribute_type_tangent:
				tanc++;
				break;
			default:
				pc++;
				printf("\n^Found some wacky attribute: %s",p.attributes[i].name);
				break;
		}
	}
	if(pc && nc){
		switch (tc){
			case 0:
				return attribute_combo_pn;
			case 1:
				return attribute_combo_pnt;
			case 2:
				return attribute_combo_pntt;
			}
	}
	return attribute_combo_invalid;
}

int main()
{

	init_glfw();
	GLFWwindow* window = create_glfw_window();
	init_glad();

	unsigned int shaderProgram;
	{
		char vertexShaderCode[FILE_SIZE_SHADER_MAX];
		char fragmentShaderCode[FILE_SIZE_SHADER_MAX];
		read_string_from_disk("/home/parth/code/psychC/shaders/vertex.vs",vertexShaderCode);
		read_string_from_disk("/home/parth/code/psychC/shaders/fragment.fs",fragmentShaderCode);
		unsigned int vertexShader = compile_shader(vertexShaderCode, GL_VERTEX_SHADER);
		unsigned int fragmentShader = compile_shader(fragmentShaderCode, GL_FRAGMENT_SHADER);
		shaderProgram = create_program(vertexShader,fragmentShader);
	}


	// load mesh with it's primitives.
	unsigned int vao[MAX_PRIMITIVE_COUNT_PER_MESH];
	unsigned int ebo[MAX_PRIMITIVE_COUNT_PER_MESH];
	unsigned int vbo[MAX_PRIMITIVE_COUNT_PER_MESH];
	unsigned int indices_size[MAX_PRIMITIVE_COUNT_PER_MESH];
	int indice_type[MAX_PRIMITIVE_COUNT_PER_MESH];

	const char* model_path = "models/DamagedHelmet.gltf";
	{
		cgltf_options options = {0};
		cgltf_data* data = NULL;
		cgltf_result result = cgltf_parse_file(&options,model_path, &data);
		if (result == cgltf_result_success)
			result = cgltf_load_buffers(&options, data, model_path);
		{
			// let's pick first mesh for now

			int primitive_count = data->meshes[0].primitives_count; 
			glGenVertexArrays(primitive_count, vao);
			glGenBuffers(primitive_count, vbo);
			glGenBuffers(primitive_count, ebo);

			for ( int i = 0; i < primitive_count; i++)
			{

				cgltf_primitive p = data->meshes[0].primitives[i];

				// load indices
				cgltf_buffer_view* buf_view = p.indices->buffer_view;
				indices_size[0] = p.indices->count;
				indice_type[0] = cgltf_to_gl_type_indices[p.indices->component_type];

				printf("\nIndices: %li, type: %i",indices_size,p.indices->component_type);

				glBindVertexArray(vao[i]); 
				glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo[i]);
				glBufferData(GL_ELEMENT_ARRAY_BUFFER, buf_view->size ,buf_view->buffer->data + buf_view->offset,GL_STATIC_DRAW);

				// load attributes
				attribute_combo combo_type = get_attributes_present(p);
				float* pos_buf = NULL, *norm_buf = NULL;
				float* texcoord_buf[2] = {NULL,NULL};
				size_t pos_size,norm_size;
				size_t texcoord_size[2];
				int attribute_count = p.attributes_count;

				int texcoord_index = 0;
				for( int j =0 ; j< attribute_count;j++){
					void* att_buf;
					size_t att_size;
					cgltf_accessor *accessor = p.attributes[j].data;
					cgltf_buffer_view* buf_view = accessor->buffer_view;
					printf("\nAttribute %s of type %i stored in offset: %li, count:%li, stride: %li",accessor->name, accessor->component_type, accessor->offset, accessor->count, accessor->stride);
					if( accessor->component_type == cgltf_component_type_r_32f){
						att_buf = buf_view->buffer->data + buf_view->offset;
						att_size = buf_view->size;
					}else{
						crash_game("\nEncountered non-float vertex data");
					}
					switch (p.attributes[j].type) {
						case cgltf_attribute_type_position:
							pos_buf = att_buf;
							pos_size = att_size;
							break;
						case cgltf_attribute_type_normal:
							norm_buf = att_buf;
							norm_size = att_size;
							break;
						case cgltf_attribute_type_texcoord:
							texcoord_buf[texcoord_index] = att_buf;
							texcoord_size[texcoord_index++] = att_size;
							break;
						default:
							printf("\n^Found some wacky attribute: %s",p.attributes[j].name);
							break;
					}
				}
				
				void* bdata = NULL;
				size_t bsize;
				switch(combo_type){
					case attribute_combo_pn:
						{
							assert(pos_size == norm_size);
							int vertice_count = pos_size/(sizeof(float)*3);
							bsize = sizeof(struct vertice_pn)*(vertice_count);
							struct vertice_pn* vertex = malloc(bsize);
							bdata = vertex;
							for(int i = 0; i < vertice_count; i++){
								vertex[i].position[0] = *(pos_buf + (3 * i));
								vertex[i].position[1] = *(pos_buf + (3 * i) + 1);
								vertex[i].position[2] = *(pos_buf + (3 * i) + 2);

								vertex[i].normal[0] = *(norm_buf + (3 * i));
								vertex[i].normal[1] = *(norm_buf + (3 * i) + 1);
								vertex[i].normal[2] = *(norm_buf + (3 * i) + 2);
							}
						}
							break;
					case attribute_combo_pnt:
						{
							assert(pos_size == norm_size);
							assert(pos_size/3 == texcoord_size[0]/2);

							int vertice_count = pos_size/(sizeof(float)*3);
							bsize = sizeof(struct vertice_pnt)*vertice_count;
							struct vertice_pnt* vertex = malloc(bsize);
							if(vertex == NULL) crash_game("couldn't malloc");

							bdata = vertex;
							for(int i = 0; i < vertice_count; i++){
								vertex[i].position[0] = *(pos_buf + (3 * i));
								vertex[i].position[1] = *(pos_buf + (3 * i) + 1);
								vertex[i].position[2] = *(pos_buf + (3 * i) + 2);

								vertex[i].normal[0] = *(norm_buf + (3 * i));
								vertex[i].normal[1] = *(norm_buf + (3 * i) + 1);
								vertex[i].normal[2] = *(norm_buf + (3 * i) + 2);

								vertex[i].texcoord[0] = *(texcoord_buf[0] + (2 * i));
								vertex[i].texcoord[1] = *(texcoord_buf[0] + (2 * i) + 1);
							}

						}
						break;
					case attribute_combo_pntt:
						{
							assert(pos_size == norm_size);
							int vertice_count = pos_size/(sizeof(float)*3);
							bsize = sizeof(struct vertice_pntt)*(pos_size/3);
							struct vertice_pntt* vertex = malloc(bsize);
							bdata = vertex;
							for(int i = 0; i < vertice_count; i++){
								vertex[i].position[0] = *(pos_buf + (3 * i));
								vertex[i].position[1] = *(pos_buf + (3 * i) + 1);
								vertex[i].position[2] = *(pos_buf + (3 * i) + 2);

								vertex[i].normal[0] = *(norm_buf + (3 * i));
								vertex[i].normal[1] = *(norm_buf + (3 * i) + 1);
								vertex[i].normal[2] = *(norm_buf + (3 * i) + 2);

								vertex[i].texcoord[0] = *(texcoord_buf[0] + (2 * i));
								vertex[i].texcoord[1] = *(texcoord_buf[0] + (2 * i) + 1);

								vertex[i].texcoord1[0] = *(texcoord_buf[1] + (2 * i));
								vertex[i].texcoord1[1] = *(texcoord_buf[1] + (2 * i) + 1);
							}
						}
						break;
					case attribute_combo_invalid:
						crash_game("Attribute/s missing");
						break;
				}

				glBindBuffer(GL_ARRAY_BUFFER,vbo[i]);
				glBufferData(GL_ARRAY_BUFFER, bsize,bdata,GL_STATIC_DRAW);
				free(bdata);

				switch(combo_type){
					case attribute_combo_pn:
							glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(struct vertice_pn), (void*)0);
							glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(struct vertice_pn), (void*)(sizeof(float)*3));
							break;
					case attribute_combo_pnt:
							glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(struct vertice_pnt), (void*)0);
							glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(struct vertice_pnt), (void*)(sizeof(float)*3));
							glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(struct vertice_pnt), (void*)(sizeof(float)*6));
							break;
					case attribute_combo_pntt:
							glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(struct vertice_pntt), (void*)0);
							glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(struct vertice_pntt), (void*)(sizeof(float)*3));
							glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(struct vertice_pntt), (void*)(sizeof(float)*6));
							glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, sizeof(struct vertice_pntt), (void*)(sizeof(float)*8));
							break;

				}


				glEnableVertexAttribArray(0);
				glBindBuffer(GL_ARRAY_BUFFER, 0); 
				glBindVertexArray(0); 

				}

			}
				printf("\n omg is it failing on  cgltf_free ????");
				fflush(stdout);
		cgltf_free(data);
				printf("\n it was not.");
				fflush(stdout);
	}
				printf("\n Then where ????");
				fflush(stdout);


//  // set up vertex data (and buffer(s)) and configure vertex attributes
//  // ------------------------------------------------------------------
//  float vertices[] = {
//       0.5f,  0.5f, 0.0f,  // top right
//       0.5f, -0.5f, 0.0f,  // bottom right
//      -0.5f, -0.5f, 0.0f,  // bottom left
//      -0.5f,  0.5f, 0.0f   // top left 
//  };
//  unsigned int indices[] = {  // note that we start from 0!
//      0, 1, 3,  // first Triangle
//      1, 2, 3   // second Triangle
//  };

//  unsigned int VBO, VAO, EBO;
//  glGenVertexArrays(1, &VAO);
//  glGenBuffers(1, &VBO);
//  glGenBuffers(1, &EBO);

//  glBindVertexArray(VAO);

//  glBindBuffer(GL_ARRAY_BUFFER, VBO);
//  glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

//  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
//  glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

//  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
//  glEnableVertexAttribArray(0);
//  glBindBuffer(GL_ARRAY_BUFFER, 0); 
//  glBindVertexArray(0); 


    // uncomment this call to draw in wireframe polygons.
    //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    // render loop
    // -----------
    while (!glfwWindowShouldClose(window))
    {
        // input
        // -----
        processInput(window);
        // render
        // ------
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        // draw our first triangle
        glUseProgram(shaderProgram);
        glBindVertexArray(vao[0]); // seeing as we only have a single VAO there's no need to bind it every time, but we'll do so to keep things a bit more organized
        glDrawElements(GL_TRIANGLES, indices_size[0], indice_type[0], 0);
        // glBindVertexArray(0); // no need to unbind it every time 
 
        // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
        // -------------------------------------------------------------------------------
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // optional: de-allocate all resources once they've outlived their purpose:
    // ------------------------------------------------------------------------
    glDeleteVertexArrays(1, &vao[0]);
    glDeleteBuffers(1, &vbo[0]);
    glDeleteBuffers(1, &ebo[0]);
    glDeleteProgram(shaderProgram);

    // glfw: terminate, clearing all previously allocated GLFW resources.
    // ------------------------------------------------------------------
    glfwTerminate();
	printf("\nbye!\n");
    return 0;
}

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void processInput(GLFWwindow *window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow *window, int width, int height)
{
    // make sure the viewport matches the new window dimensions; note that width and 
    // height will be significantly larger than specified on retina displays.
    glViewport(0, 0, width, height);
}


