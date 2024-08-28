#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include "disk.h"
#include "constants.h"
#define CGLTF_IMPLEMENTATION
#include "cgltf.h"
// --- -- -- - -- - - DeFines move them to constants lateer -- -- -- - -- 
#define MAX_MESHES_PER_FILE 100
#define MAX_PRIMITIVES_PER_FILE 100
// -- -- -- -- -- -- Structs -- -- -- -- -- -- -- -- -- --

struct primitive {
	unsigned int vao;
	unsigned int ebo;
	unsigned int vbo;
	unsigned int indices_count;
	int gl_index_type;
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
	printf("\nGame crashed: %s\n",msg);
	fflush(stdout);
	exit(1);
}

// -- -- -- -- -- -- Cgltf parse helper functions -- -- -- -- - -
//
int cgltf_ctype_to_gl_type[7] = {GL_INVALID_VALUE,GL_BYTE,GL_UNSIGNED_BYTE,GL_SHORT,GL_UNSIGNED_SHORT,GL_UNSIGNED_INT,GL_FLOAT};
int cgltf_ctype_to_bytes[7] = {-1,1,1,2,2,4,4};
typedef enum attribute{
	position,
	normal,
	tangent,
	texcoords,
	texcoords1,
	color,
	joints,
	weights,
	attributes_size
}attribute;


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


	// load all meshes and their primitves in this gltf file 
	int mesh_count;	
	int mesh_offset[MAX_MESHES_PER_FILE];
	int primitives_count = 0;	
	struct primitive primitives[MAX_PRIMITIVES_PER_FILE];
	const char* model_path = "models/xbot_cap.gltf";
	{
		cgltf_options options = {0};
		cgltf_data* data = NULL;
		cgltf_result result = cgltf_parse_file(&options,model_path, &data);
		if (result == cgltf_result_success)
		{
			result = cgltf_load_buffers(&options, data, model_path);
			if (result != cgltf_result_success){
				printf("cgltf couldn't load buffers : %i",result);
				crash_game("could'nt load model");
			}
			mesh_count = data->meshes_count;
			assert(mesh_count < MAX_MESHES_PER_FILE);

			for(int j =0 ; j < mesh_count; j++){
				int primitive_count = data->meshes[j].primitives_count; 
				assert(primitive_count < MAX_PRIMITIVES_PER_FILE);
				primitives_count += primitive_count;
				int pi = 0;
				for(int i = 0; i < primitive_count;i++){
					struct primitive p_primitive;
					glGenVertexArrays(1, &p_primitive.vao);
					glGenBuffers(1, &p_primitive.vbo);
					glGenBuffers(1, &p_primitive.ebo);

					cgltf_primitive p = data->meshes[j].primitives[i];

					// load indices into the ebo and call it a day
					if(p.indices  != NULL){
						cgltf_buffer_view* buf_view = p.indices->buffer_view;
						p_primitive.indices_count = p.indices->count;
						p_primitive.gl_index_type = cgltf_ctype_to_gl_type[p.indices->component_type];
						glBindVertexArray(p_primitive.vao); 
						glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, p_primitive.ebo);
						glBufferData(GL_ELEMENT_ARRAY_BUFFER, buf_view->size ,buf_view->buffer->data + buf_view->offset,GL_STATIC_DRAW);
					}else{
						crash_game("encountered un-indexed mesh data. eww.");
					}


					// find attributes

					void* attribute_buf[attributes_size] = {NULL};
					size_t attribute_size[attributes_size];
					cgltf_component_type attribute_ctype[attributes_size];
					int attribute_stride[attributes_size];
					int attribute_count = p.attributes_count;
					int texcoord_index = 0;
					int color_type = -1;
					size_t stride = 0;
					printf("\nNo. of attributes found : %i",attribute_count);
					for( int j =0 ; j< attribute_count;j++){
						cgltf_accessor *accessor = p.attributes[j].data;
						cgltf_buffer_view* buf_view = accessor->buffer_view;
						cgltf_component_type ctype = accessor->component_type;
						printf("\nAttribute %s of type %i stored in offset: %li, count:%li, stride: %li",p.attributes[j].name, accessor->component_type, accessor->offset, accessor->count, accessor->stride);
						fflush(stdout);
						assert(ctype == 2 || ctype ==4 || ctype == 6);
						stride += accessor->stride;
						void* att_buf = buf_view->buffer->data + buf_view->offset;
						size_t att_size = buf_view->size;
						cgltf_attribute_type type = p.attributes[j].type;
						if(type == cgltf_attribute_type_color){
							color_type = accessor->type;
						}
						if(type == cgltf_attribute_type_texcoord){
							attribute_buf[type-1+texcoord_index] = att_buf;
							attribute_size[type-1+texcoord_index] = att_size;
							attribute_ctype[type-1+texcoord_index] = ctype;
							attribute_stride[type-1+texcoord_index] = accessor->stride;
							texcoord_index++;
						}else if(type < cgltf_attribute_type_custom){
							attribute_buf[type-1] = att_buf;
							attribute_size[type-1] = att_size;
							attribute_ctype[type-1] = ctype;
							attribute_stride[type-1] = accessor->stride;
						}else{
							printf("\n^Found some wacky attribute: %s",p.attributes[j].name);
						}
					}

					// interleave attributes into a buffer

					int vertice_count = attribute_size[position]/(sizeof(float)*3);
					printf("\nVertice_count: %i",vertice_count);
					size_t bsize = vertice_count * stride;
					void* bdata = malloc(bsize);
					{
						void* ptr = bdata;
						for(int i =0; i< vertice_count; i++){
							for(int k =0; k < attributes_size; k++){
								if(attribute_buf[k] != NULL){
									memcpy(ptr,attribute_buf[k],attribute_stride[k]);
									ptr = (void*)((char*)ptr + attribute_stride[k]);
									attribute_buf[k] = (void*)((char*)attribute_buf[k] + attribute_stride[k]);
								}
							}
						}
					}

					glBindBuffer(GL_ARRAY_BUFFER,p_primitive.vbo);
					glBufferData(GL_ARRAY_BUFFER, bsize,bdata,GL_STATIC_DRAW);
					free(bdata);

					size_t cstride = 0; 
					for(int k =0; k < attributes_size; k++){
						if(attribute_buf[k] != NULL){
							printf("\nglVertexAttribPointer(%i,%i,%i,GL_FALSE,%i,(void*)(%i)",k, attribute_stride[k]/cgltf_ctype_to_bytes[attribute_ctype[k]], cgltf_ctype_to_gl_type[attribute_ctype[k]],stride,cstride); 
							fflush(stdout);
							glVertexAttribPointer(k, attribute_stride[k]/cgltf_ctype_to_bytes[attribute_ctype[k]], cgltf_ctype_to_gl_type[attribute_ctype[k]], GL_FALSE, stride, (void*)(cstride));
							cstride += attribute_stride[k];
						}
					}

/*
					switch(combo_type){
						case attribute_combo_pt:
							{
								int vertice_count = pos_size/(sizeof(float)*3);
								bsize = sizeof(struct vertice_pt)*(vertice_count);
								struct vertice_pt* vertex = malloc(bsize);
								bdata = vertex;
								for(int i = 0; i < vertice_count; i++){
									vertex[i].position[0] = *(pos_buf + (3 * i));
									vertex[i].position[1] = *(pos_buf + (3 * i) + 1);
									vertex[i].position[2] = *(pos_buf + (3 * i) + 2);

									vertex[i].texcoord[0] = *(texcoord_buf[0] + (2 * i));
									vertex[i].texcoord[1] = *(texcoord_buf[0] + (2 * i) + 1);
								}
							}
							break;
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
				*/


					/*
					switch(combo_type){
						case attribute_combo_pt:
							glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(struct vertice_pn), (void*)0);
							glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(struct vertice_pn), (void*)(sizeof(float)*3));
							break;
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
					*/
				glEnableVertexAttribArray(0);
				glBindBuffer(GL_ARRAY_BUFFER, 0); 
				glBindVertexArray(0); 
				primitives[pi++] =  p_primitive;
				}
				mesh_offset[j] = pi;
				}

			}
		else{
			crash_game("Please check if model exists!");
		}
		cgltf_free(data);
	}


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
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

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
		for(int i=0;i<primitives_count;i++){
			glBindVertexArray(primitives[i].vao); // seeing as we only have a single VAO there's no need to bind it every time, but we'll do so to keep things a bit more organized
			glDrawElements(GL_TRIANGLES, primitives[i].indices_count, primitives[i].gl_index_type, 0);
			//glDrawElements(GL_TRIANGLES, primitives[i].indices_count, GL_UNSIGNED_SHORT, 0);
		}
        // glBindVertexArray(0); // no need to unbind it every time 
 
        // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
        // -------------------------------------------------------------------------------
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // optional: de-allocate all resources once they've outlived their purpose:
    // ------------------------------------------------------------------------
    //glDeleteVertexArrays(1, &vao[0]);
    //glDeleteBuffers(1, &vbo[0]);
    //glDeleteBuffers(1, &ebo[0]);
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


