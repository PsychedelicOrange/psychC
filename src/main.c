#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <cglm/cglm.h>
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include "disk.h"
#include "constants.h"
#include "mesh.h"
#define CGLTF_IMPLEMENTATION
#include "cgltf.h"
// --- -- -- - -- - - Defines move them to constants lateer -- -- -- - -- 
#define MAX_MESHES_PER_FILE 100
#define MAX_PRIMITIVES_PER_FILE 100
#define MAX_BONES_PER_MESH 100
#define MAX_PRIMITIVES_PER_MESH 100
#define VBOS_ENV 3


// -- -- -- -- -- -- Function declare -- -- -- -- -- --- --


struct mesh load_mesh(cgltf_mesh* cmesh);
struct primitive load_primitive(cgltf_primitive p);

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

// -- -- -- -- -- -- Structs -- -- -- -- -- --- --

typedef struct buffer{
	void*  data;
	size_t size;
}buffer;

typedef struct drawable_mesh{
	unsigned int vao;
	size_t indices_count;
}drawable_mesh;

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

void load_primitives_env (cgltf_mesh* mesh, primitive_env* primitives, size_t primitive_index){
	size_t mpriv_count = mesh->primitives_count;
	for(size_t i = 0; i < mpriv_count;i++){
		primitive_env p;
		size_t attribute_count = mesh->primitives[i].attributes_count;
		cgltf_attribute* attributes = mesh->primitives[i].attributes;
		p.vertices = malloc(sizeof(vertex_env)*attributes[0].data->count);
		p.vertex_count = attributes[0].data->count;
		int ti=0;

		// load indices 
		if(mesh->primitives[i].indices != NULL){
			cgltf_accessor* indices = mesh->primitives[0].indices;
			assert(indices->component_type == cgltf_component_type_r_16u);
			cgltf_buffer_view* buf_view = indices->buffer_view;
			printf("\n INDICES:");
			printf("\n\t indices.ctype:\t %i",indices->component_type);
			printf("\n\t indices.type:\t %i",indices->type);
			printf("\n\t indices.offset:\t%li",indices->offset);
			printf("\n\t indices.count:\t%li",indices->count);
			printf("\n\t indices.stride:\t%li",indices->stride);
			printf("\n\t\t buffer.name:\t%s",buf_view->buffer->name);
			printf("\n\t\t buffer_view.offset:\t%li",buf_view->offset);
			printf("\n\t\t buffer_view.size:\t%li",buf_view->size);
			p.indices_count = indices->count;
			p.indices = malloc(sizeof(unsigned short)*p.indices_count);
			memcpy(p.indices,buf_view->buffer->data + buf_view->offset + indices->offset,buf_view->size);
		}else{
			crash_game("encountered un-indexed mesh data. eww.");
		}
		

		for(size_t j =0; j < attribute_count;j++){

			cgltf_accessor *accessor = attributes[j].data;
			cgltf_buffer_view* buf_view = accessor->buffer_view;

			printf("\n Attribute %s",attributes[j].name);
			printf("\n\t Accessor.ctype:\t %i",accessor->component_type);
			printf("\n\t Accessor.type:\t %i",accessor->type);
			printf("\n\t Accessor.offset:\t%li",accessor->offset);
			printf("\n\t Accessor.count:\t%li",accessor->count);
			printf("\n\t Accessor.stride:\t%li",accessor->stride);
			printf("\n\t\t buffer.name:\t%s",buf_view->buffer->name);
			printf("\n\t\t buffer_view.offset:\t%li",buf_view->offset);
			printf("\n\t\t buffer_view.size:\t%li",buf_view->size);
			fflush(stdout);

			void* att_buf = buf_view->buffer->data + buf_view->offset + accessor->offset;
			size_t att_size = buf_view->size;

			switch(attributes[j].type){
				case cgltf_attribute_type_position:
					assert(accessor->component_type == cgltf_component_type_r_32f);
					assert(accessor->type == 3);
					printf("\n Attribute loading : %s",attributes[j].name);
					for(size_t k = 0; k < p.vertex_count; k++){
						memcpy(p.vertices[k].position,att_buf,sizeof(float)*3);
						att_buf = (void*) ((char*)att_buf + accessor->stride);
					}
					break;
				case cgltf_attribute_type_normal:
					assert(accessor->component_type == cgltf_component_type_r_32f);
					assert(accessor->type == 3);
					printf("\n Attribute loading : %s",attributes[j].name);
					for(size_t k = 0; k < p.vertex_count; k++){
						memcpy(p.vertices[k].normal,att_buf,sizeof(float)*3);
						att_buf = (void*) ((char*)att_buf + accessor->stride);
					}
					break;
				case cgltf_attribute_type_tangent:
					assert(accessor->component_type == cgltf_component_type_r_32f);
					assert(accessor->type == 4);
					printf("\n Attribute loading : %s",attributes[j].name);
					for(size_t k = 0; k < p.vertex_count; k++){
						memcpy(p.vertices[k].tangent,att_buf,sizeof(float)*4);
						att_buf = (void*) ((char*)att_buf + accessor->stride);
					}
					break;
				case cgltf_attribute_type_texcoord:
					assert(accessor->component_type == cgltf_component_type_r_32f);
					assert(accessor->type == 2);
					printf("\n Attribute loading : %s",attributes[j].name);
					if(ti++){
						for(size_t k = 0; k < p.vertex_count; k++){
							memcpy(p.vertices[k].uv,att_buf,sizeof(float)*2);
							att_buf = (void*) ((char*)att_buf + accessor->stride);
						}
					}else{
						for(size_t k = 0; k < p.vertex_count; k++){
							memcpy(p.vertices[k].uv1,att_buf,sizeof(float)*2);
							att_buf = (void*) ((char*)att_buf + accessor->stride);
						}
					}
					break;
				default:
					printf("Environment mesh doesn't support : %s",attributes[j].name);
			}
			fflush(stdout);	
		}
		for(int i = 0; i < p.vertex_count;i++){
			float* n = p.vertices[i].normal;
			printf("\nNORMALS: %f,%f,%f",n[0],n[1],n[2]);
		}
		primitives[primitive_index++] = p;
	}
}
 size_t load_model_env(char* model_path, primitive_env* primitives){
		cgltf_options options = {0};
		cgltf_data* data = NULL;
		cgltf_result result = cgltf_parse_file(&options,model_path, &data);
		size_t primitive_count = 0;
		if (result == cgltf_result_success)
		{
			result = cgltf_load_buffers(&options, data, model_path);
			if (result != cgltf_result_success){
				printf("cgltf couldn't load buffers : %i",result);
				crash_game("could'nt load model");
			}
			size_t meshes_count = data->meshes_count;
			cgltf_mesh* meshes = data->meshes;
			for(int i=0; i< meshes_count; i++){
				load_primitives_env(meshes,primitives,primitive_count);
				primitive_count += meshes->primitives_count;
			}

		}else{
			crash_game("Please check if model exists!");
		}
		cgltf_free(data);
		return primitive_count;
 }

buffer append_primitive_vertice_data(primitive_env* primitives,size_t from,size_t to){
	printf("lol %zu to %zu",from,to);
	vertex_env* vertices;
	size_t vertice_count =0;
	for(int i = from; i < to; i++){
		vertice_count += primitives[i].vertex_count;
	}
	vertices = malloc(sizeof(vertex_env)*vertice_count);
	vertex_env* ptr = vertices;
	for(int i = from; i < to; i++){
		memcpy(ptr,primitives[i].vertices,primitives[i].vertex_count*sizeof(vertex_env));
		ptr += primitives[i].vertex_count;
	}
	buffer buf = {vertices,vertice_count*sizeof(vertex_env)};
	return buf;
}

buffer append_primitive_indice_data(primitive_env* primitives,size_t from,size_t to){
	printf("lol %zu to %zu",from,to);
	unsigned short* indices;
	size_t indices_count = 0;
	for(int i = from; i < to; i++){
		indices_count += primitives[i].indices_count;
	}
	indices = malloc(sizeof(unsigned short)*indices_count);
	unsigned short* ptr = indices;
	for(int i = from; i < to; i++){
		memcpy(ptr,primitives[i].indices,primitives[i].indices_count*sizeof(unsigned short));
		ptr += primitives[i].indices_count;
	}
	buffer buf = {indices,indices_count*sizeof(unsigned short)};
	return buf;
}

drawable_mesh load_single_primitive_env(primitive_env p){
	// load model primitive into buffers and return ids
	unsigned int ebo,vbo;
	drawable_mesh m = {0};
	m.indices_count = p.indices_count;
	glGenVertexArrays(1,&m.vao);
	glGenBuffers(1, &vbo);
	glGenBuffers(1, &ebo);

	glBindVertexArray(m.vao); 
	glBindBuffer(GL_ARRAY_BUFFER,vbo);
	glBufferData(GL_ARRAY_BUFFER, p.vertex_count*sizeof(vertex_env),p.vertices,GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,ebo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, p.indices_count*sizeof(unsigned short),p.indices,GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);	
	glVertexAttribPointer(0,3,GL_FLOAT, GL_FALSE, sizeof(vertex_env), (void*)(offsetof(vertex_env,position)));
	glEnableVertexAttribArray(1);	
	glVertexAttribPointer(1,3,GL_FLOAT, GL_FALSE, sizeof(vertex_env), (void*)(offsetof(vertex_env,normal)));
	glEnableVertexAttribArray(2);	
	glVertexAttribPointer(2,4,GL_FLOAT, GL_FALSE, sizeof(vertex_env), (void*)(offsetof(vertex_env,tangent)));
	glEnableVertexAttribArray(3);	
	glVertexAttribPointer(3,2,GL_FLOAT, GL_FALSE, sizeof(vertex_env), (void*)(offsetof(vertex_env,uv)));
	glEnableVertexAttribArray(4);	
	glVertexAttribPointer(4,2,GL_FLOAT, GL_FALSE, sizeof(vertex_env), (void*)(offsetof(vertex_env,uv1)));

	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0); 
	glBindVertexArray(0); 
	return m;
}

void draw_single_primitive_env(unsigned int shaderProgram,drawable_mesh d){
	glUseProgram(shaderProgram);
	glBindVertexArray(d.vao);
	glDrawElements(GL_TRIANGLES, d.indices_count,GL_UNSIGNED_SHORT, 0);
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

	// load primitives for env model into memory
	primitive_env primitives[MAX_PRIMITIVES_PER_FILE];
	size_t primitive_count = load_model_env("models/cube.gltf",primitives);
	
	printf("\nPrimitives count %li",primitive_count);

	// load models into ogl buffer
	drawable_mesh mesh =load_single_primitive_env(primitives[0]);
	// divide all primitives for env models into batches of vbos
	unsigned int vbos_env[VBOS_ENV];
	unsigned int vaos_env[VBOS_ENV];
	unsigned int ebos_env[VBOS_ENV];
	size_t indice_count[VBOS_ENV] = {0};
	size_t batch_range[VBOS_ENV] = {0};
	size_t batch_rangei = 0;
	{
		size_t batch_size = 1000000; // 1cr vertices
		size_t cb = 0;

		int i = 0;
		while(i < primitive_count){
			size_t cvc = 0;
			for(;i < primitive_count; i++){
				cvc += primitives[i].vertex_count;
				if(cvc > batch_size)
					break;
				indice_count[i] += primitives[i].indices_count;
			}
			batch_range[++batch_rangei] = i;
		}

	}
	// generate buffers, arrange env data and load the batches into vbos 
	//
	glGenVertexArrays(batch_rangei,vaos_env);
	glGenBuffers(batch_rangei, vbos_env);
	glGenBuffers(batch_rangei, ebos_env);
	for(size_t i  = 0; i< batch_rangei; i++){

		glBindVertexArray(vaos_env[i]); 
		buffer vertices = append_primitive_vertice_data(primitives,batch_range[i],batch_range[i+1]);
		glBindBuffer(GL_ARRAY_BUFFER,vbos_env[i]);
		glBufferData(GL_ARRAY_BUFFER, vertices.size,vertices.data,GL_STATIC_DRAW);
		free(vertices.data);
		
		buffer indices = append_primitive_indice_data(primitives,batch_range[i],batch_range[i+1]);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,ebos_env[i]);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size,indices.data,GL_STATIC_DRAW);
		free(indices.data);

		glEnableVertexAttribArray(0);	
		glVertexAttribPointer(0,3,GL_FLOAT, GL_FALSE, sizeof(vertex_env), (void*)(offsetof(vertex_env,position)));
		glEnableVertexAttribArray(1);	
		glVertexAttribPointer(1,3,GL_FLOAT, GL_FALSE, sizeof(vertex_env), (void*)(offsetof(vertex_env,normal)));
		glEnableVertexAttribArray(2);	
		glVertexAttribPointer(2,4,GL_FLOAT, GL_FALSE, sizeof(vertex_env), (void*)(offsetof(vertex_env,tangent)));
		glEnableVertexAttribArray(3);	
		glVertexAttribPointer(3,2,GL_FLOAT, GL_FALSE, sizeof(vertex_env), (void*)(offsetof(vertex_env,uv)));
		glEnableVertexAttribArray(4);	
		glVertexAttribPointer(4,2,GL_FLOAT, GL_FALSE, sizeof(vertex_env), (void*)(offsetof(vertex_env,uv1)));

		glEnableVertexAttribArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, 0); 
		glBindVertexArray(0); 

	}

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

		// draw envs

		draw_single_primitive_env(shaderProgram,mesh);

		/*
		glUseProgram(shaderProgram);
		for(size_t i  = 0; i< batch_rangei; i++){
			glBindVertexArray(vaos_env[i]);
			glDrawElements(GL_TRIANGLES, indice_count[i],GL_UNSIGNED_SHORT, 0);
		}
		*/
	
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


