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
#define MAX_PRIMITIVES 1000
#define MAX_BONES_PER_MESH 100
#define MAX_PRIMITIVES_PER_MESH 100


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

// in out structs aka convinience structs :)
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
	const GLubyte* vendor = glGetString(GL_VENDOR); // Returns the vendor
	const GLubyte* renderer = glGetString(GL_RENDERER); // Returns a hint to the model
	printf("\nVendor: %s",vendor);
	printf("\nRenderer: %s",renderer);
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
void print_indices(cgltf_accessor* indices){
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
}
void print_accessor(cgltf_accessor *accessor ){
	cgltf_buffer_view* buf_view = accessor->buffer_view;
	printf("\n\t Accessor.ctype:\t %i",accessor->component_type);
	printf("\n\t Accessor.type:\t %i",accessor->type);
	printf("\n\t Accessor.offset:\t%li",accessor->offset);
	printf("\n\t Accessor.count:\t%li",accessor->count);
	printf("\n\t Accessor.stride:\t%li",accessor->stride);
	printf("\n\t\t buffer.name:\t%s",buf_view->buffer->name);
	printf("\n\t\t buffer_view.offset:\t%li",buf_view->offset);
	printf("\n\t\t buffer_view.size:\t%li",buf_view->size);
}
void load_primitives_actor (cgltf_mesh* mesh, primitive_actor* primitives, size_t primitive_index){
	size_t mpriv_count = mesh->primitives_count;
	for(size_t i = 0; i < mpriv_count;i++){
		primitive_actor p;
		size_t attribute_count = mesh->primitives[i].attributes_count;
		cgltf_attribute* attributes = mesh->primitives[i].attributes;
		p.vertices = malloc(sizeof(vertex_env)*attributes[0].data->count);
		p.vertex_count = attributes[0].data->count;

		// load indices 
		if(mesh->primitives[i].indices != NULL){
			cgltf_accessor* indices = mesh->primitives[0].indices;
			assert(indices->component_type == cgltf_component_type_r_16u);
			cgltf_buffer_view* buf_view = indices->buffer_view;
			p.indices_count = indices->count;
			p.indices = malloc(sizeof(unsigned short)*p.indices_count);
			memcpy(p.indices,buf_view->buffer->data + buf_view->offset + indices->offset,buf_view->size);
		}else{
			crash_game("encountered un-indexed mesh data. eww.");
		}
		
		for(size_t j =0; j < attribute_count;j++){

			cgltf_accessor *accessor = attributes[j].data;
			cgltf_buffer_view* buf_view = accessor->buffer_view;

			void* att_buf = buf_view->buffer->data + buf_view->offset + accessor->offset;
			size_t att_size = buf_view->size;

			switch(attributes[j].type){
				case cgltf_attribute_type_position:
					assert(accessor->component_type == cgltf_component_type_r_32f);
					assert(accessor->type == 3);
					for(size_t k = 0; k < p.vertex_count; k++){
						memcpy(p.vertices[k].position,att_buf,sizeof(float)*3);
						att_buf = (void*) ((char*)att_buf + accessor->stride);
					}
					break;
				case cgltf_attribute_type_normal:
					assert(accessor->component_type == cgltf_component_type_r_32f);
					assert(accessor->type == 3);
					for(size_t k = 0; k < p.vertex_count; k++){
						memcpy(p.vertices[k].normal,att_buf,sizeof(float)*3);
						att_buf = (void*) ((char*)att_buf + accessor->stride);
					}
					break;
				case cgltf_attribute_type_tangent:
					assert(accessor->component_type == cgltf_component_type_r_32f);
					assert(accessor->type == 4);
					for(size_t k = 0; k < p.vertex_count; k++){
						memcpy(p.vertices[k].tangent,att_buf,sizeof(float)*4);
						att_buf = (void*) ((char*)att_buf + accessor->stride);
					}
					break;
				case cgltf_attribute_type_texcoord:
					assert(accessor->component_type == cgltf_component_type_r_32f);
					assert(accessor->type == 2);
					for(size_t k = 0; k < p.vertex_count; k++){
						memcpy(p.vertices[k].uv,att_buf,sizeof(float)*2);
						att_buf = (void*) ((char*)att_buf + accessor->stride);
					}
					break;
				case cgltf_attribute_type_joints:
					assert(accessor->component_type == cgltf_component_type_r_16u);
					assert(accessor->type == 4);
					for(size_t k = 0; k < p.vertex_count; k++){
						memcpy(p.vertices[k].uv,att_buf,sizeof(unsigned short)*4);
						att_buf = (void*) ((char*)att_buf + accessor->stride);
					}
					break;
				case cgltf_attribute_type_weights:
					assert(accessor->component_type == cgltf_component_type_r_32f);
					assert(accessor->type == 4);
					for(size_t k = 0; k < p.vertex_count; k++){
						memcpy(p.vertices[k].uv,att_buf,sizeof(float)*4);
						att_buf = (void*) ((char*)att_buf + accessor->stride);
					}
					break;
				default:
					printf("Environment mesh doesn't support : %s",attributes[j].name);
			}
			fflush(stdout);	
		}
		primitives[primitive_index++] = p;
	}
}

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
			cgltf_accessor* indices = mesh->primitives[i].indices;
			assert(indices->component_type == cgltf_component_type_r_16u);
			cgltf_buffer_view* buf_view = indices->buffer_view;
			p.indices_count = indices->count;
			p.indices = malloc(sizeof(unsigned short)*p.indices_count);
			memcpy(p.indices,buf_view->buffer->data + buf_view->offset + indices->offset,buf_view->size);
		}else{
			crash_game("encountered un-indexed mesh data. eww.");
		}
		
		for(size_t j =0; j < attribute_count;j++){

			cgltf_accessor *accessor = attributes[j].data;
			cgltf_buffer_view* buf_view = accessor->buffer_view;

			void* att_buf = buf_view->buffer->data + buf_view->offset + accessor->offset;
			size_t att_size = buf_view->size;

			switch(attributes[j].type){
				case cgltf_attribute_type_position:
					assert(accessor->component_type == cgltf_component_type_r_32f);
					assert(accessor->type == 3);
					for(size_t k = 0; k < p.vertex_count; k++){
						memcpy(p.vertices[k].position,att_buf,sizeof(float)*3);
						att_buf = (void*) ((char*)att_buf + accessor->stride);
					}
					break;
				case cgltf_attribute_type_normal:
					assert(accessor->component_type == cgltf_component_type_r_32f);
					assert(accessor->type == 3);
					for(size_t k = 0; k < p.vertex_count; k++){
						memcpy(p.vertices[k].normal,att_buf,sizeof(float)*3);
						att_buf = (void*) ((char*)att_buf + accessor->stride);
					}
					break;
				case cgltf_attribute_type_tangent:
					assert(accessor->component_type == cgltf_component_type_r_32f);
					assert(accessor->type == 4);
					for(size_t k = 0; k < p.vertex_count; k++){
						memcpy(p.vertices[k].tangent,att_buf,sizeof(float)*4);
						att_buf = (void*) ((char*)att_buf + accessor->stride);
					}
					break;
				case cgltf_attribute_type_texcoord:
					assert(accessor->component_type == cgltf_component_type_r_32f);
					assert(accessor->type == 2);
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
		primitives[primitive_index++] = p;
	}
}

size_t load_model_actor(char* model_path, primitive_actor* primitives, size_t primitive_index){
	assert(primitive_index < MAX_PRIMITIVES);
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
		size_t meshes_count = data->meshes_count;
		cgltf_mesh* meshes = data->meshes;
		for(int i=0; i< meshes_count; i++){
			load_primitives_actor(meshes,primitives,primitive_index);
			primitive_index += meshes->primitives_count;
		}

	}else{
		crash_game("Please check if model exists!");
	}
	cgltf_free(data);
	return primitive_index;
}

size_t load_model_env(char* model_path, primitive_env* primitives, size_t primitive_index){
	assert(primitive_index < MAX_PRIMITIVES);
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
		size_t meshes_count = data->meshes_count;
		cgltf_mesh* meshes = data->meshes;
		for(int i=0; i< meshes_count; i++){
			load_primitives_env(meshes,primitives,primitive_index);
			primitive_index += meshes->primitives_count;
		}

	}else{
		crash_game("Please check if model exists!");
	}
	cgltf_free(data);
	return primitive_index;
 }

// ---- - -- -- -- ---- -- -- -- upload ogl  helper functions -- -- -- -- -- -- -- 

buffer append_primitive_vertice_data(primitive_env* primitives,size_t from,size_t to){
	vertex_env* vertices;
	size_t vertice_count =0;
	for(int i = from; i < to; i++){
		primitives[i].base_vertex = vertice_count;
		vertice_count += primitives[i].vertex_count;
	}
	printf("\nTotal vertice count for environment mesh: %li",vertice_count);
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
	unsigned short* indices;
	size_t indices_count = 0;
	for(int i = from; i < to; i++){
		primitives[i].index_index = indices_count;
		indices_count += primitives[i].indices_count;
	}
	printf("\nTotal indice count for environment mesh: %li",indices_count);
	indices = malloc(sizeof(unsigned short)*indices_count);
	unsigned short* ptr = indices;
	for(int i = from; i < to; i++){
		memcpy(ptr,primitives[i].indices,primitives[i].indices_count*sizeof(unsigned short));
		ptr += primitives[i].indices_count;
	}
	buffer buf = {indices,indices_count*sizeof(unsigned short)};
	return buf;
}
void vertexAttrib_env(){
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
}

void vertexAttrib_actor(){
	glEnableVertexAttribArray(0);	
	glVertexAttribPointer(0,3,GL_FLOAT, GL_FALSE, sizeof(vertex_actor), (void*)(offsetof(vertex_actor,position)));
	glEnableVertexAttribArray(1);	
	glVertexAttribPointer(1,3,GL_FLOAT, GL_FALSE, sizeof(vertex_actor), (void*)(offsetof(vertex_actor,normal)));
	glEnableVertexAttribArray(2);	
	glVertexAttribPointer(2,4,GL_FLOAT, GL_FALSE, sizeof(vertex_actor), (void*)(offsetof(vertex_actor,tangent)));
	glEnableVertexAttribArray(3);	
	glVertexAttribPointer(3,2,GL_FLOAT, GL_FALSE, sizeof(vertex_actor), (void*)(offsetof(vertex_actor,uv)));
	glEnableVertexAttribArray(4);	
	glVertexAttribPointer(4,4,GL_UNSIGNED_SHORT, GL_FALSE, sizeof(vertex_actor), (void*)(offsetof(vertex_actor,joints)));
	glEnableVertexAttribArray(5);	
	glVertexAttribPointer(4,4,GL_FLOAT, GL_FALSE, sizeof(vertex_actor), (void*)(offsetof(vertex_actor,weights)));
}

drawable_mesh upload_single_primitive_actor(primitive_actor p){
	// upload single primtive into ogl buffer
	// load model primitive into buffers and return ids
	unsigned int ebo,vbo;
	drawable_mesh m = {0};
	m.indices_count = p.indices_count;
	glGenVertexArrays(1,&m.vao);
	glGenBuffers(1, &vbo);
	glGenBuffers(1, &ebo);

	glBindVertexArray(m.vao); 
	glBindBuffer(GL_ARRAY_BUFFER,vbo);
	glBufferData(GL_ARRAY_BUFFER, p.vertex_count*sizeof(vertex_actor),p.vertices,GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,ebo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, p.indices_count*sizeof(unsigned short),p.indices,GL_STATIC_DRAW);
	vertexAttrib_actor();
	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0); 
	glBindVertexArray(0); 
	return m;
}
drawable_mesh upload_single_primitive_env(primitive_env p){
	// upload single primtive into ogl buffer
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
	vertexAttrib_env();
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

void draw_multiple_primitive_env(unsigned int shaderProgram,unsigned int env_vao,primitive_env* primitives, size_t primitive_count){
	glUseProgram(shaderProgram);
	glBindVertexArray(env_vao);
	for(int i = 0; i < primitive_count;i++){
		//printf("\nglDrawElementsBaseVertex(GL_TRIANGLES, %li, GL_UNSIGNED_SHORT,(void*) %li, %li)",primitives[i].indices_count, (primitives[i].index_index), primitives[i].base_vertex);
		glDrawElementsBaseVertex(GL_TRIANGLES, primitives[i].indices_count, GL_UNSIGNED_SHORT, (void*)(primitives[i].index_index * 2), primitives[i].base_vertex);
	}
}

unsigned int upload_multiple_primitive_env(primitive_env* primitives, size_t primitive_count){
	// smash env mesh into a single buffer, 
	unsigned int vbos_env;
	unsigned int vaos_env;
	unsigned int ebos_env;
	// generate buffers, arrange env data and load the batches into vbos 
	//
	glGenVertexArrays(1,&vaos_env);
	glGenBuffers(1, &vbos_env);
	glGenBuffers(1, &ebos_env);

	glBindVertexArray(vaos_env); 
	buffer vertices = append_primitive_vertice_data(primitives,0,primitive_count);
	printf("Size of vertices buffer: %li",vertices.size);
	glBindBuffer(GL_ARRAY_BUFFER,vbos_env);
	glBufferData(GL_ARRAY_BUFFER, vertices.size,vertices.data,GL_STATIC_DRAW);
	free(vertices.data);

	buffer indices = append_primitive_indice_data(primitives,0,primitive_count);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,ebos_env);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size,indices.data,GL_STATIC_DRAW);
	free(indices.data);

	vertexAttrib_env();

	glBindBuffer(GL_ARRAY_BUFFER, 0); 
	glBindVertexArray(0); 

	return vaos_env;
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
	primitive_env primitives_env[MAX_PRIMITIVES];
	size_t primitive_count_env = load_model_env("/mnt/Windows/Data/Models/sponza/sponza_bckup.gltf",primitives_env,0);
	printf("\nPrimitives_env count %li",primitive_count_env);

	// load primitives for actors into mem
	primitive_actor primitives_actor[MAX_PRIMITIVES];
	//size_t primitive_count_actor = load_model_actor("models/simple_skin.gltf",primitives_actor,0);

	unsigned int env_vao =  upload_multiple_primitive_env(primitives_env,primitive_count_env);

	drawable_mesh mesh = upload_single_primitive_actor(primitives_actor[0]);

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

		//draw_single_primitive_env(shaderProgram,mesh);
		draw_multiple_primitive_env(shaderProgram,env_vao, primitives_env,primitive_count_env);

		// draw actors
	
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


