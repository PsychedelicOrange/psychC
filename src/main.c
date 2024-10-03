#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <cglm/cglm.h>
#include <cglm/struct.h>
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#define CGLTF_IMPLEMENTATION
#include "disk.h"
#include "constants.h"
#include "mesh.h"
#include "boiler.h"
#include "shader.h"
#include "env.h"
// --- -- -- - -- - - Defines move them to constants lateer -- -- -- - -- 
#define MAX_MESHES_PER_FILE 100

// -- -- -- -- -- -- Function declare -- -- -- -- -- --- --
void framebuffer_size_callback(GLFWwindow *window, int width, int height);

// -- -- -- -- -- -- Engine Helpers Functions -- -- -- -- -- --- --
void crash_game(char* msg){
	printf("\nGame crashed: %s\n",msg);
	fflush(stdout);
	exit(1);
}

// -- -- -- -- -- -- debug print math functions -- -- -- -- - -
//
void print_vec3(float* vec){
	printf("(%f,%f,%f)",vec[0],vec[1],vec[2]);
}
void print_vec4(float* vec){
	printf("\r(%f,%f,%f,%f)",vec[0],vec[1],vec[2],vec[3]);
}
void print_mat4_ptr(float* mat){
	printf("\n[\n");
	for(int i =0; i < 4; i++){
		for(int j =0; j < 4; j++){
			printf("%f,",mat[(4*i)+j]);
		}
		printf("\n");
	}
	printf("]\n");
}
void print_mat4(float mat[4][4]){
	printf("\n[\n");
	for(int i =0; i < 4; i++){
		for(int j =0; j < 4; j++){
			printf("%f,",mat[i][j]);
		}
		printf("\n");
	}
	printf("\n]\n");
}

// -- -- -- -- -- -- Cgltf parse helper functions -- -- -- -- - - int cgltf_ctype_to_gl_type[7] = {GL_INVALID_VALUE,GL_BYTE,GL_UNSIGNED_BYTE,GL_SHORT,GL_UNSIGNED_SHORT,GL_UNSIGNED_INT,GL_FLOAT};
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
void print_vertex_actor(vertex_actor a){
	//printf("\nfloat position[3]:");
	//print_vec3(a.position);
	//printf("\nfloat normal[3]:");
	//print_vec3(a.normal);
	//printf("\nfloat tangent[4]:");
	//print_vec4(a.tangent);
	printf("\nunsigned short joints[4]:");
	printf("(%u,%u,%u,%u)",a.joints[0],a.joints[1],a.joints[2],a.joints[3]);
	//printf("\nfloat weights[4]:");
	//print_vec4(a.weights);
}

void print_mesh_actor(mesh_actor mesh_actor){
	printf("\n Mesh:");
	printf("\n Skin:");
	printf("\n\t No. of joints:\t %li",mesh_actor.joints_count);
	for(size_t i  = 0 ; i < mesh_actor.joints_count;i++){
		printf("\n\tJoint \t %li:",i);
		printf("\n\t\tIndex \t %i:",mesh_actor.joints[i].index);
		printf("\n\t\tTranslation \t :");
		print_vec3(mesh_actor.joints[i].translation);
		printf("\n\t\tRotation \t :");
		print_vec4(mesh_actor.joints[i].rotation.raw);
		printf("\n\t\tChildren \t (%li) :",mesh_actor.joints[i].children_count);
		printf("[");
		for(size_t j = 0; j < mesh_actor.joints[i].children_count;j++){
			printf("%i,",mesh_actor.joints[i].children[j]);
		}
		printf("]");
	}
}

void load_primitives_actor (cgltf_mesh* mesh, mesh_actor* emesh){
	size_t mpriv_count = mesh->primitives_count;
	for(size_t i = 0; i < mpriv_count;i++){
		primitive_actor p;
		size_t attribute_count = mesh->primitives[i].attributes_count;
		cgltf_attribute* attributes = mesh->primitives[i].attributes;
		p.vertices = malloc(sizeof(vertex_actor)*attributes[0].data->count);
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
					assert(accessor->component_type == cgltf_component_type_r_8u || accessor->component_type == cgltf_component_type_r_16u);
					assert(accessor->type == 4);
					if(accessor->component_type == cgltf_component_type_r_8u ){
						for(int i = 0 ; i < p.vertex_count;i++){
							p.vertices[i].joints[0]= (unsigned short)(((char*)att_buf)[0]);
							p.vertices[i].joints[1]= (unsigned short)(((char*)att_buf)[1]);
							p.vertices[i].joints[2]= (unsigned short)(((char*)att_buf)[2]);
							p.vertices[i].joints[3]= (unsigned short)(((char*)att_buf)[3]);
							att_buf = (void*) ((char*)att_buf + accessor->stride);
						}
					}else{
						for(size_t k = 0; k < p.vertex_count; k++){
							memcpy(p.vertices[k].joints,att_buf,sizeof(unsigned short)*4);
							att_buf = (void*) ((char*)att_buf + accessor->stride);
						}
					}
					break;
				case cgltf_attribute_type_weights:
					assert(accessor->component_type == cgltf_component_type_r_32f);
					assert(accessor->type == 4);
					for(size_t k = 0; k < p.vertex_count; k++){
						memcpy(p.vertices[k].weights,att_buf,sizeof(float)*4);
						att_buf = (void*) ((char*)att_buf + accessor->stride);
					}
					break;
				default:
					printf("\nActor mesh doesn't support : %s",attributes[j].name);
			}
			fflush(stdout);	
		}
		for(int i =0; i < p.vertex_count; i++){
			print_vertex_actor(p.vertices[i]);
		}
		emesh->primitives[i] = p;
	}
}


joint load_joint(cgltf_data* data, cgltf_node* jnode){
	joint j;
	if(jnode->name != NULL)
		j.name = strdup(jnode->name);

	if(jnode->has_translation){
		memcpy(j.translation,jnode->translation,sizeof(float)*3);
	}else{
		memset(j.translation,0,sizeof(float)*3);
	}

	if(jnode->has_rotation){
		memcpy(j.rotation.raw,jnode->rotation,sizeof(float)*4);
	}else{
		memset(j.rotation.raw,0,sizeof(float)*3);
		j.rotation.raw[3] = 1;
	}

	j.index = jnode - data->nodes;
	j.children_count = jnode->children_count;
	for(int i = 0; i < j.children_count; i++){
		if(jnode == jnode->children[i]) assert(false);
		j.children[i] = jnode->children[i] - data->nodes;
	}
	return j;
}

void load_ibm(cgltf_skin* skin ,mesh_actor* mesh){
	cgltf_accessor *accessor = skin->inverse_bind_matrices;
	cgltf_buffer_view* buf_view = accessor->buffer_view;
	printf("\nIBM accessor:");
	print_accessor(accessor);
	assert(accessor->component_type == cgltf_component_type_r_32f);
	assert(accessor->type == cgltf_type_mat4);

	void* att_buf = buf_view->buffer->data + buf_view->offset + accessor->offset;
	size_t att_size = buf_view->size;
	for(size_t k = 0; k < accessor->count; k++){
		memcpy(mesh->joints[k].inverseBindMatrice.raw ,att_buf,sizeof(float)*16);
		print_mat4_ptr((float*)att_buf);
		att_buf = (void*) ((char*)att_buf + accessor->stride);
	}
}

void load_skin(cgltf_data* data,cgltf_skin* skin, mesh_actor* mesh){
	for(int i = 0 ; i  < skin->joints_count; i++)
	{
		mesh->joints[i] = load_joint(data,skin->joints[i]);
	}
	// load the inverseBindMatrices
	load_ibm(skin,mesh);
	// load weights and morph targets here in future
}

size_t getBytesPerElement(cgltf_accessor* acc){
	cgltf_component_type ctype = acc->component_type;
	cgltf_type type = acc->type;
	int cgltf_ctype_to_bytes[] = {-1,1,1,2,2,4,4};
	int cgltf_type_to_count[]= {-1,1,2,3,4,4,9,16};	
	return cgltf_ctype_to_bytes[ctype]*type;
}
buffer load_accessor(cgltf_accessor* acc){
	buffer buf;
	cgltf_buffer_view* buf_view = acc->buffer_view;
	printf("\n Loading accessor");
	print_accessor(acc);
	void* att_buf = buf_view->buffer->data + buf_view->offset + acc->offset;
	size_t att_size = buf_view->size;
	// get size of individual attribute from types;
	int elSize = getBytesPerElement(acc);
	buf.size = elSize * acc->count;
	buf.data = malloc(elSize* acc->count);

	printf("\n\tAccessor element size in bytes: %i",elSize);
	for(int i = 0; i < acc->count; i++){
		memcpy((void*)((char*)buf.data+(elSize*i)),att_buf,elSize);
		att_buf = (void*) ((char*)att_buf + acc->stride);
	}
	return buf;
}

sampler load_sampler(cgltf_animation_sampler* gsamp){
	sampler sampler;
	sampler.interpolation = (interpolation)gsamp->interpolation;
	sampler.keyframes = load_accessor(gsamp->input);
	sampler.data = load_accessor(gsamp->output);
	sampler.element_size = getBytesPerElement(gsamp->output);

//for(int i =0; i < sampler.keyframes.size / 4; i++){
//	printf("\nKeyframe %i at time: %f",i,((float*)sampler.keyframes.data)[i]);
//	printf("\tRotation: ( %f, %f, %f, %f) ",((float*)sampler.data.data)[4*i],((float*)sampler.data.data)[4*i + 1],((float*)sampler.data.data)[4*i + 2],((float*)sampler.data.data)[4*i +3]);
//}

	return sampler;
}

void load_animations(cgltf_data* data,mesh_actor* mesh){
	int anim_count = data->animations_count;
	assert( anim_count < 10);
	mesh->animations_count = anim_count;
	for(int i = 0; i < anim_count; i++){
		printf("\n Animation found: %s",data->animations[i].name);
		animation animation;
		animation.channels_count = data->animations[i].channels_count;
		animation.channels = malloc(sizeof(channel)*animation.channels_count);
		for(int c = 0;c < animation.channels_count; c ++){
			cgltf_animation_channel channel = data->animations[i].channels[c];	
			animation.channels[c].property = channel.target_path - 1;
			animation.channels[c].sampler = load_sampler(channel.sampler);
			// store address of animated property in the data_ptr for easy update
			for(int j = 0; j < mesh->joints_count; j++){
				if(mesh->joints[j].index == channel.target_node - data->nodes){
					if(animation.channels[c].property == prop_translation){
						animation.channels[c].data_ptr = mesh->joints[j].translation;
						break;
					}
					if(animation.channels[c].property == prop_rotation){
						animation.channels[c].data_ptr = mesh->joints[j].rotation.raw;
						break;
					}
				}
			}
		}
		mesh->animations[i] = animation;
	}
}

size_t load_model_actor(char* model_path, mesh_actor* meshes, size_t meshes_index){
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

		// load meshes and associate skin
		size_t nodes_count = data->nodes_count;
		cgltf_node* nodes = data->nodes;
		for(int i=0; i< nodes_count; i++){
			if(nodes[i].mesh != NULL){
				mesh_actor mesh_actor;
				mesh_actor.primitives_count = nodes[i].mesh->primitives_count;
				mesh_actor.primitives = malloc(sizeof(primitive_actor)*mesh_actor.primitives_count);
				load_primitives_actor(nodes[i].mesh,&mesh_actor);

				if(nodes[i].skin != NULL){
					printf("\n Skin: %s",nodes[i].skin->name);
					printf("\n\t Joints_count : %li",nodes[i].skin->joints_count);
					fflush(stdout);
					mesh_actor.joints_count = nodes[i].skin->joints_count;
					mesh_actor.joints = malloc(sizeof(joint)*mesh_actor.joints_count);
					load_skin(data,nodes[i].skin,&mesh_actor);
					load_animations(data,&mesh_actor);
					print_mesh_actor(mesh_actor);
				}
				meshes[meshes_index++] = mesh_actor;
			}
		}

	}else{
		crash_game("Please check if model exists!");
	}
	cgltf_free(data);
	return meshes_index;
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
	glVertexAttribPointer(5,4,GL_FLOAT, GL_FALSE, sizeof(vertex_actor), (void*)(offsetof(vertex_actor,weights)));
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

void calc_joint_matrices(float* matrices,mesh_actor mesh_actor){
	size_t jc = mesh_actor.joints_count;
	float* ptr = matrices;
	for(int i =0; i < jc; i++){
		joint* j = &mesh_actor.joints[i];
		// calculate local transform
		mat4 m;
		glm_translate_make(m,j->translation);

		mat4 rotated;
		glm_quat_rotate(m,j->rotation.raw,rotated);
		mat4s matmul;
	 	glm_mat4_mul(rotated,j->inverseBindMatrice.raw,m);
		memcpy(ptr,m,sizeof(float)*16);
		//mat_mul(ptr,matrix,&(j->inverseBindMatrice[0][0]));
		//printf("\nFinal matrix transform for joint %i",i);
		//print_mat4(m);
		// no need to transform children! this is just skinning !
	ptr += 16;}
	
	//fflush(stdout);
}

int get_previous_index(float currentTime,buffer buffer){
	// binary search for fun
	float* array = buffer.data;
	int size = buffer.size/sizeof(float);
	int high = size - 1, low = 0;
	int pred = low - 1;
	int succ = high + 1;
	while(low <= high){
		int i = (high+low)/2;
		if(array[i] < currentTime){
			pred = i;
			low = i+1;
		}else if(array[i] > currentTime){
			succ = i;
			high = i - 1;
		}else return i;
	}return pred;
}

int main()
{

	init_glfw();
	GLFWwindow* window = (GLFWwindow*)create_glfw_window(800,600);
	init_glad();

	unsigned int environmentShader;
	{
		char vertexShaderCode[FILE_SIZE_SHADER_MAX];
		char fragmentShaderCode[FILE_SIZE_SHADER_MAX];
		read_string_from_disk("/home/parth/code/psychC/shaders/env.vs",vertexShaderCode);
		read_string_from_disk("/home/parth/code/psychC/shaders/env.fs",fragmentShaderCode);
		unsigned int vertexShader = compile_shader(vertexShaderCode, GL_VERTEX_SHADER);
		unsigned int fragmentShader = compile_shader(fragmentShaderCode, GL_FRAGMENT_SHADER);
		environmentShader = create_program(vertexShader,fragmentShader);
	}
	unsigned int actorShader;
	{
		char vertexShaderCode[FILE_SIZE_SHADER_MAX];
		char fragmentShaderCode[FILE_SIZE_SHADER_MAX];
		read_string_from_disk("/home/parth/code/psychC/shaders/actor.vs",vertexShaderCode);
		read_string_from_disk("/home/parth/code/psychC/shaders/actor.fs",fragmentShaderCode);
		unsigned int vertexShader = compile_shader(vertexShaderCode, GL_VERTEX_SHADER);
		unsigned int fragmentShader = compile_shader(fragmentShaderCode, GL_FRAGMENT_SHADER);
		actorShader = create_program(vertexShader,fragmentShader);
	}


	// load primitives for actors into mem
	mesh_actor* mesh_actors = malloc(sizeof(mesh_actor)*MAX_MESHES_PER_FILE);
	size_t meshes_count = load_model_actor("models/simple_skin.gltf",mesh_actors,0);
	printf("\nNo. of meshes: %li",meshes_count);
	// upload primitives
	drawable_mesh d = upload_single_primitive_actor(mesh_actors[0].primitives[0]);

    // uncomment this call to draw in wireframe polygons.
    //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

	float* joint_matrices = malloc(sizeof(float)*mesh_actors[0].joints_count*16);
	
	calc_joint_matrices(joint_matrices,mesh_actors[0]);
    // render loop
    // -----------
    while (!glfwWindowShouldClose(window))
    {
        // input
        // -----
		if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		{
			glfwSetWindowShouldClose(window, true);
		}
		if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
		{
			mesh_actors[0].animations[0].started = glfwGetTime();
			//printf("\n");
		}
			
        // render
        // ------
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

		// animate - aka change the joint orientations
		{
			if(mesh_actors[0].animations[0].started > 0 ){ 
				for(int i =0; i < mesh_actors[0].animations[0].channels_count; i++){
					channel* channel = &(mesh_actors[0].animations[0].channels[i]);
					sampler samp = channel->sampler;
					switch(samp.interpolation){
						case STEP:
							{
								float currentTime = glfwGetTime() - mesh_actors[0].animations[0].started;
								int previousTimeIndex = get_previous_index(currentTime,samp.keyframes);
								memcpy(channel->data_ptr,&samp.data.data[samp.element_size*previousTimeIndex],samp.element_size);
							}
							break;
						case LINEAR:
							{
								float currentTime = glfwGetTime() - mesh_actors[0].animations[0].started;
								int previousTimeIndex = get_previous_index(currentTime,samp.keyframes);
								float* keyframes =  samp.keyframes.data;
								float interpolationValue = (currentTime - keyframes[previousTimeIndex]) / (keyframes[previousTimeIndex+1] - keyframes[previousTimeIndex]);
								switch(channel->property){
									case prop_weights:
									case prop_rotation:
										{
											versors p,n;
											memcpy(p.raw, ((char*)samp.data.data + samp.element_size*previousTimeIndex),samp.element_size);
											memcpy(n.raw, ((char*)samp.data.data + samp.element_size*(previousTimeIndex+1)),samp.element_size);
											versors slerped_value = glms_quat_slerp(p,n,interpolationValue);
											memcpy(channel->data_ptr,slerped_value.raw,sizeof(versors));
										}
										break;
									case prop_scale:
									case prop_translation:
										{
											vec3s p,n;
											memcpy(p.raw, ((char*)samp.data.data + samp.element_size*previousTimeIndex),samp.element_size);
											memcpy(n.raw, ((char*)samp.data.data + samp.element_size*(previousTimeIndex+1)),samp.element_size);
											vec3s lerped_value = glms_vec3_lerp(p,n,interpolationValue);
											memcpy(channel->data_ptr,lerped_value.raw,sizeof(vec3s));
										}
										break;
								}
							}
							break;
						case CUBICSPLINE:
							{
								//todo
							}
							break;
					}
				}
			}
		}

		// calc joint matrices from joint orientations and upload to shader
		calc_joint_matrices(joint_matrices,mesh_actors[0]);
		int jointmat_loc = glGetUniformLocation(actorShader, "u_jointMat");
		glUseProgram(actorShader);
		glUniformMatrix4fv(jointmat_loc,mesh_actors[0].joints_count,GL_FALSE,joint_matrices);	
		// draw actors
		draw_single_primitive_env(actorShader,d);
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
    glDeleteProgram(environmentShader);

    // glfw: terminate, clearing all previously allocated GLFW resources.
    // ------------------------------------------------------------------
    glfwTerminate();
	printf("\nbye!\n");
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


