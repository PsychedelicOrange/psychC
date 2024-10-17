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
#include "util.h"
#include "camera.h"
// --- -- -- - -- - - Defines -- -- -- - -- -- -- -- -- -- 
const vec3s up = {{0,1,0}};
float lastX,lastY;
camera cam;

// -- -- -- -- -- -- Function declare -- -- -- -- -- --- --
void framebuffer_size_callback(GLFWwindow *window, int width, int height);
static void cursor_position_callback(GLFWwindow* window, double xpos, double ypos);

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
	printf("(%f,%f,%f,%f)",vec[0],vec[1],vec[2],vec[3]);
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
	printf("\nfloat position[3]:");
	print_vec3(a.position);
	printf("\nfloat normal[3]:");
	print_vec3(a.normal);
	printf("\nfloat tangent[4]:");
	print_vec4(a.tangent);
	printf("\nunsigned short joints[4]:");
	printf("(%u,%u,%u,%u)",a.joints[0],a.joints[1],a.joints[2],a.joints[3]);
	printf("\nfloat weights[4]:");
	print_vec4(a.weights);
}

void print_sampler(sampler samp){
	printf("\nsampler print");
	printf("\nsamp.keyframes: %li",samp.keyframes.size);
	printf("\nsamp.data: %li",samp.data.size);
	printf("\nsamp.element: %li",samp.element_size);
	printf("\nsamp.interpolation: %i",samp.interpolation);
}
void print_joint(joint* j){
		printf("\n\tJoint \t :");
		printf("\n\t\tIndex \t %i:",j->gltf_index);
		printf("\n\t\tTranslation \t :");
		print_vec3(j->translation);
		printf("\n\t\tRotation \t :");
		print_vec4(j->rotation.raw);
}
void print_mesh_actor(mesh_actor mesh_actor){
	printf("\n Mesh:");
	printf("\n Skin:");
	//printf("\n\t No. of joints:\t %li",mesh_actor.joints_count);
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
	////for(int i =0; i < p.vertex_count; i++){
	////	print_vertex_actor(p.vertices[i]);
	////}
		emesh->primitives[i] = p;
	}
}


joint* load_joint(cgltf_data* data, cgltf_node* jnode){
	joint* j = malloc(sizeof(joint));
	if(jnode->name != NULL)
		j->name = strdup(jnode->name);

	if(jnode->has_translation){
		memcpy(j->translation,jnode->translation,sizeof(float)*3);
	}else{
		memset(j->translation,0,sizeof(float)*3);
	}

	if(jnode->has_rotation){
		memcpy(j->rotation.raw,jnode->rotation,sizeof(float)*4);
	}else{
		memset(j->rotation.raw,0,sizeof(float)*3);
		j->rotation.raw[3] = 1;
	}

	j->parent = NULL;
	j->gltf_index = jnode - data->nodes;
	/*
	j->children_count = jnode->children_count;
	for(int i = 0; i < j->children_count; i++){
		if(jnode == jnode->children[i]) assert(false);
		j->children[i] = jnode->children[i] - data->nodes;
	}
	*/
	return j;
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
	//printf("\n Loading accessor");
	//print_accessor(acc);
	void* att_buf = buf_view->buffer->data + buf_view->offset + acc->offset;
	size_t att_size = buf_view->size;
	// get size of individual attribute from types;
	int elSize = getBytesPerElement(acc);
	buf.size = elSize * acc->count;
	buf.data = malloc(elSize* acc->count);

	//printf("\n\tAccessor element size in bytes: %i",elSize);
	for(int i = 0; i < acc->count; i++){
		memcpy((void*)((char*)buf.data+(elSize*i)),att_buf,elSize);
		att_buf = (void*) ((char*)att_buf + acc->stride);
	}
	return buf;
}

void load_ibm(cgltf_skin* cskin ,skin* skin){
	cgltf_accessor *accessor = cskin->inverse_bind_matrices;
	cgltf_buffer_view* buf_view = accessor->buffer_view;
	//printf("\nIBM accessor:");
	//print_accessor(accessor);
	assert(accessor->component_type == cgltf_component_type_r_32f);
	assert(accessor->type == cgltf_type_mat4);
	// try this ig ?
	//load_accessor(accessor);

	void* att_buf = buf_view->buffer->data + buf_view->offset + accessor->offset;
	size_t att_size = buf_view->size;
	for(size_t k = 0; k < accessor->count; k++){
		memcpy(skin->inverseBindMatrices[k].raw ,att_buf,sizeof(float)*16);
		//print_mat4_ptr((float*)att_buf);
		att_buf = (void*) ((char*)att_buf + accessor->stride);
	}
}

sampler load_sampler(cgltf_animation_sampler* gsamp){
	sampler sampler;
	sampler.interpolation = (interpolation)gsamp->interpolation;
	sampler.keyframes = load_accessor(gsamp->input);
	sampler.data = load_accessor(gsamp->output);
	sampler.element_size = getBytesPerElement(gsamp->output);
//	print_sampler(sampler);
//	fflush(stdout);

	return sampler;
}

void load_animations(cgltf_data* data,model_actor* model){
	int anim_count = data->animations_count;
	assert(anim_count < 10);
	model->animations_count = anim_count;

	for(int i = 0; i < anim_count; i++){
		//printf("\n Animation found: %s",data->animations[i].name);
		animation animation;
		animation.started = -1;
		animation.channels_count = data->animations[i].channels_count;
		animation.channels = malloc(sizeof(channel)*animation.channels_count);
		for(int c = 0;c < animation.channels_count; c ++){
			cgltf_animation_channel channel = data->animations[i].channels[c];	
			animation.channels[c].property = channel.target_path - 1;
			animation.channels[c].sampler = load_sampler(channel.sampler);
			// store address of animated property in the data_ptr for easy update
			joint* joint = hashmap_get(&model->joints,channel.target_node - data->nodes);
			assert(joint);
			if(animation.channels[c].property == prop_translation){
				animation.channels[c].data_ptr = joint->translation;
			}else if(animation.channels[c].property == prop_rotation){
				animation.channels[c].data_ptr = joint->rotation.raw;
			}else if(animation.channels[c].property == prop_scale){
				//printf("\nIgnoring scale animation for bones.");
			}else{
				assert(0);
			}
		}
		model->animations[i] = animation;
	}
	for(int k = 0;k < model->animations[0].channels_count;k++){
		assert(model->animations[0].channels[k].sampler.element_size != 0);
	}
}
void topo_sort(cgltf_data* data, cgltf_node* joint, int* visited, int* vsize, int* topo_order, int* index){
	for(int i = 0; i < joint->children_count;i++){
		int isvisited= 0;
		for(int v = 0; v < *vsize; v++){
			if(visited[v] == joint->children[i] - data->nodes){
				isvisited = 1;
			}
		}
		if(isvisited) continue;
		visited[(*vsize)++] = joint->children[i]- data->nodes;
		topo_sort(data,joint->children[i],visited,vsize,topo_order,index);
	}
	topo_order[(*index)++] = joint - data->nodes;
}
buffer calc_joint_topo_order(cgltf_data* data,cgltf_skin skin){
	int jc = skin.joints_count;
	buffer topo_order;
	topo_order.data = malloc(sizeof(int)*jc); // stack
	topo_order.size = sizeof(int)*jc;
	int* topo_order_ar = (int*)topo_order.data;
	int index = 0;
	int* visited = malloc(sizeof(int)*jc); 
	int vindex = 0;
	memset(visited,-1,sizeof(int)*jc);
	for(int i =0; i < jc;i++){
		int isvisited= 0;
		for(int v = 0; v < vindex; v++){
			if(visited[v] == skin.joints[i] - data->nodes){
				isvisited = 1;
			}
		}
		if(isvisited) continue;
		visited[vindex++] = skin.joints[i] - data->nodes;
		topo_sort(data,skin.joints[i],visited,&vindex,topo_order_ar,&index);
	}
	printf("\n Topological order for joint: \n");
	for(int i = 0; i < jc;i ++){
		printf("%i, ",topo_order_ar[i]);
	}
	fflush(stdout);
	free(visited);
	return topo_order;
}
model_actor load_model_actor(char* model_path){
	model_actor model;
	cgltf_options options = {0}; 
	cgltf_data* data = NULL;
	cgltf_result result = cgltf_parse_file(&options,model_path, &data);
	model.joints.size = 0;

	if (result == cgltf_result_success)
	{
		result = cgltf_load_buffers(&options, data, model_path);
		if (result != cgltf_result_success){
			printf("cgltf couldn't load buffers : %i",result);
			crash_game("could'nt load model");
		}

		//load_meshes
		{
			model.meshes_count = data->meshes_count;
			model.meshes = malloc(sizeof(mesh_actor) * model.meshes_count);
			for(int i = 0;i < model.meshes_count; i++){
				mesh_actor mesh_actor;
				mesh_actor.skin_ref = NULL;
				mesh_actor.primitives_count = data->meshes[i].primitives_count;
				mesh_actor.primitives = malloc(sizeof(primitive_actor)*mesh_actor.primitives_count);
				load_primitives_actor(&(data->meshes[i]),&mesh_actor);
				model.meshes[i] = mesh_actor;
			}
		}

		// load skins
		{
			model.skins_count = data->skins_count;
			model.skins = malloc(sizeof(skin) * model.skins_count);
			for(int i = 0;i < model.skins_count; i++){
				model.skins[i].topo_order = calc_joint_topo_order(data,data->skins[i]);
				load_ibm(&data->skins[i],&model.skins[i]);
				model.skins[i].joints_count = data->skins[i].joints_count;
				assert(model.skins[i].joints_count < 100);
				for(int j =0; j <model.skins[i].joints_count;j++){
					int key = data->skins[i].joints[j] - data->nodes; 
					joint* search = hashmap_get(&model.joints,key);
					if(search == NULL){
						joint* joint = load_joint(data,data->skins[i].joints[j]);
						hashmap_put(&model.joints,key,joint);
						model.skins[i].joint_refs[j] = joint;
					}
				}
				// populate joint->parent for all joints
				{
					for(int j = 0; j < model.skins[i].joints_count;j++){
						cgltf_node* cjo= data->skins[i].joints[j];
						joint* parent = hashmap_get(&model.joints, cjo - data->nodes);
						for(int c = 0; c < cjo->children_count; c++){
							joint* child = hashmap_get(&model.joints, cjo->children[c] - data->nodes);
							if(!child)continue;
							child->parent = parent; 
						}
					}
				}
				// debug joints
				{
		////		printf("\nDebug joints:");
		////		for(int j =0; j < model.joints.size;j++){
		////			printf("\n key: %i", model.joints.map[j].key);
		////			joint* joint = model.joints.map[j].value;
		////			print_joint(joint);
		////		}
				}
			}
		}
		// populate refs to skins in meshes
		{ 
			size_t nodes_count = data->nodes_count;
			cgltf_node* nodes = data->nodes;
			for(int i=0; i< nodes_count; i++){
				if(nodes[i].mesh != NULL && nodes[i].skin != NULL){
					model.meshes[nodes[i].mesh - data->meshes].skin_ref = &(model.skins[nodes[i].skin - data->skins]);
				}
			}
		}

		// load animations and store refs to joints
		load_animations(data,&model);
	}else{
		crash_game("Please check if model exists!");
	}
	cgltf_free(data);
	return model;
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

drawable_prim upload_single_primitive_actor(primitive_actor p){
	// upload single primtive into ogl buffer
	// load model primitive into buffers and return ids
	unsigned int ebo,vbo;
	drawable_prim m = {0};
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

void calc_joint_matrices(float* matrices,hashmap_int* joints,drawable_mesh* dmesh){
	skin* skin = dmesh->skin_ref;
	size_t jc = skin->joints_count;

///	for(int j =0; j < joints->size;j++){
///		printf("\n key: %i", joints->map[j].key);
///		joint* joint = joints->map[j].value;
///		print_joint(joint);
///	}

	// calc global transforms for each joint (in topo order so its simple)
	int* order = ((int*)skin->topo_order.data);
	for (int i = (skin->topo_order.size/sizeof(int)) - 1; i >= 0 ; i--){
		joint* j = hashmap_get(joints,order[i]);
		if(!j->parent){
			mat4 m;
			glm_translate_make(m,j->translation);
			glm_quat_rotate(m,j->rotation.raw,j->transform.raw);
			//printf("\nLocal Transforms:# %i",j->gltf_index);
		//	print_mat4(j->transform.raw);
			fflush(stdout);
		}else{
				mat4 m;
				glm_translate_make(m,j->translation);
				mat4 rotated;
				glm_quat_rotate(m,j->rotation.raw,rotated);
				glm_mat4_mul(j->parent->transform.raw,rotated,j->transform.raw);
			//	printf("\nGlobalTransform:# %i",j->gltf_index);
			//	print_mat4(j->transform.raw);
				fflush(stdout);
			}
		}
	// print global_transform
	{
	////printf("\nGlobal Transforms: *");
	////for(int i =0; i < jc; i++){
	////	joint* j = skin->joint_refs[i];
	////	print_mat4(j->transform.raw);
	////}
	}

	float* ptr = matrices;
	for(int i =0; i < jc; i++){
		joint* j = skin->joint_refs[i];
		// calculate joint matrices 
		// globalTransformOfSelf = (globalTransformOfParent * localTransformOfSelf)
		mat4s m = GLMS_MAT4_IDENTITY_INIT;
		glm_mat4_mul(j->transform.raw,skin->inverseBindMatrices[i].raw,m.raw);
		memcpy(ptr,m.raw,sizeof(float)*16);
		//mat_mul(ptr,matrix,&(j->inverseBindMatrice[0][0]));
		//printf("\nFinal matrix transform for joint %i",i);
		//print_mat4(m);
		//no need to transform children! this is forward kinematices !
		ptr += 16;
	}
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
	}return pred > 0? pred : 0;
}

/* animate model a.k.a change joints transform */
void animate(drawable_model* model){
	for(int j = 0; j < model->animations_count;j++){
		if(model->animations[j].started > 0 ){ 
			for(int i =0; i < model->animations[j].channels_count; i++){
				channel* channel = &(model->animations[j].channels[i]);
				if(channel->property == prop_scale)continue;
				sampler samp = channel->sampler;
				switch(samp.interpolation){
					case STEP:
						{
							float currentTime = glfwGetTime() - model->animations[j].started;
							int previousTimeIndex = get_previous_index(currentTime,samp.keyframes);
							if(currentTime > ((float *)samp.keyframes.data)[samp.keyframes.size/sizeof(float)-1]){
								model->animations[j].started = -1;
							}
							memcpy(channel->data_ptr,&(samp.data.data[samp.element_size*previousTimeIndex]),samp.element_size);
						}
						break;
					case LINEAR:
						{
							float currentTime = glfwGetTime() - model->animations[j].started;
							int previousTimeIndex = get_previous_index(currentTime,samp.keyframes);
							if(currentTime > ((float *)samp.keyframes.data)[samp.keyframes.size/sizeof(float)-1]){
								model->animations[j].started = -1;
							}
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
									break;
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
							assert(0);
						}
						break;
				}
			}
		}
	}
}

drawable_model upload_model_actor(model_actor* model){
	drawable_model dmodel;
	dmodel.animations_count = model->animations_count;
	memcpy(dmodel.animations,model->animations,sizeof(animation)*model->animations_count);
	dmodel.joints = &model->joints;
	dmodel.meshes = malloc(sizeof(drawable_mesh)*(model->meshes_count));
	dmodel.meshes_count = model->meshes_count;
	for(int i = 0 ; i < model->meshes_count; i++){
		mesh_actor* mesh = &(model->meshes[i]);
		drawable_mesh dmesh;
		dmesh.skin_ref = mesh->skin_ref;
		dmesh.jointMatricesData = malloc(sizeof(float) * dmesh.skin_ref->joints_count  * 16);
		dmesh.prims = malloc(mesh->primitives_count * sizeof(drawable_prim));;
		dmesh.prims_count = mesh->primitives_count;
		for(int j = 0 ; j < mesh->primitives_count; j++){
			dmesh.prims[j] = upload_single_primitive_actor(mesh->primitives[j]);
		}
		dmodel.meshes[i] = dmesh;
	}
	return dmodel;
}

void draw_model(drawable_model* dmodel,unsigned int shaderProgram){
	for(int i = 0; i < dmodel->meshes_count; i++){
		// TODO combine vbos for all vbos in future ( check materials flow ?)
		drawable_mesh* mesh_actor = &dmodel->meshes[i];
		if(mesh_actor->skin_ref != NULL){
			calc_joint_matrices(mesh_actor->jointMatricesData,(dmodel->joints),mesh_actor);
			int jointmat_loc = glGetUniformLocation(shaderProgram, "u_jointMat");
			glUseProgram(shaderProgram);
			glUniformMatrix4fv(jointmat_loc,mesh_actor->skin_ref->joints_count,GL_FALSE,mesh_actor->jointMatricesData);	
		}
		for(int p = 0 ; p < mesh_actor->prims_count;p++){
			draw_single_primitive_env(shaderProgram,mesh_actor->prims[p]);
		}
	}
}

int main(int argc, char *argv[])
{

	init_glfw();
	GLFWwindow* window = (GLFWwindow*)create_glfw_window(800,600);
	lastX = 800.0f/2;
	lastY = 600.0f/2;
	// callbacks and settings on window
	if (glfwRawMouseMotionSupported())
		glfwSetInputMode(window, GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);
	glfwSetCursorPosCallback(window, cursor_position_callback);
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	//glfwSetKeyCallback(window,key_callback);


	cam = init_camera();

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

	{
		mat4s projection = glms_perspective(glm_rad(45.0f), (float)800 / (float)600, 0.1f, 10000.0f);
		mat4s cube_transform = GLMS_MAT4_IDENTITY_INIT;
		setUniformMat4(actorShader,projection,"projection");
		setUniformMat4(actorShader,cube_transform,"model");
	}

	// load primitives for actors into mem
	model_actor model = load_model_actor(argv[1]);

	for(int i = 0; i < model.animations[0].channels_count;i++){
		if(model.animations[0].channels[i].sampler.element_size == 0){
		printf("\nCaught!");
		fflush(stdout);
		assert(model.animations[0].channels[i].sampler.element_size != 0);
		}
	}

	drawable_model dmodel = upload_model_actor(&model);
	// upload primitives
	// drawable_prim d = upload_single_primitive_actor(model.meshes[0].primitives[0]);
	// drawable_prim d2 = upload_single_primitive_actor(model.meshes[1].primitives[0]);

    // uncomment this call to draw in wireframe polygons.
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
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
			dmodel.animations[0].started = glfwGetTime();
		}
		if( GLFW_PRESS == glfwGetKey(window, GLFW_KEY_LEFT_SHIFT)){
			cam.position.y -= 0.1;
		}
		if( GLFW_PRESS == glfwGetKey(window, GLFW_KEY_W)){
			cam.position.z -= 0.1;
		}
		if( GLFW_PRESS == glfwGetKey(window, GLFW_KEY_A)){
			cam.position.x -= 0.1;
		}
		if( GLFW_PRESS == glfwGetKey(window, GLFW_KEY_S)){
			cam.position.z += 0.1;
		}
		if( GLFW_PRESS == glfwGetKey(window, GLFW_KEY_D)){
			cam.position.x += 0.1;
		}
		update_first_person_camera(&cam);
			
        // render
        // ------
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

		// draw actors
		animate(&dmodel);
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
