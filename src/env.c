#include "env.h"
#include "log.h"
#include <glad/glad.h>
#include <assert.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "cgltfhelper.h"


void load_primitives_env (cgltf_mesh* mesh, primitive_env* primitives, size_t primitive_index){
	size_t mpriv_count = mesh->primitives_count;
	logd("no. of primitives in current mesh: %i", mpriv_count);
	for(size_t i = 0; i < mpriv_count;i++){
		primitive_env p;
		size_t attribute_count = mesh->primitives[i].attributes_count;
		cgltf_attribute* attributes = mesh->primitives[i].attributes;
		logd("Vertex count for primitive: %li",attributes[0].data->count);
		fflush(stdout);
		p.vertices = malloc(sizeof(vertex_env)*attributes[0].data->count);
		p.vertex_count = attributes[0].data->count;
		int ti=0;

		// load indices 
		if(mesh->primitives[i].indices != NULL){
		    cgltf_accessor* indices = mesh->primitives[i].indices;
		    assert(indices->component_type == cgltf_component_type_r_16u);
		    buffer buf = load_accessor(indices);
		    p.indices_count = indices->count;
		    p.indices = buf.data;
		    assert(p.indices_count == buf.size/sizeof (unsigned short));
		}else{
		    loge("encountered un-indexed mesh data. eww.");
		    exit(1);
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
					logw("Environment mesh doesn't support : %s",attributes[j].name);
			}
			fflush(stdout);	
		}
		primitives[primitive_index++] = p;
		logd("primitive %i loaded",i);
	}
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

size_t load_model_env(char* model_path, primitive_env* primitives, size_t primitive_index){
	assert(primitive_index < MAX_PRIMITIVES);
	cgltf_options options = {0};
	cgltf_data* data = NULL;
	cgltf_result result = cgltf_parse_file(&options,model_path, &data);
	if (result == cgltf_result_success)
	{
		result = cgltf_load_buffers(&options, data, model_path);
		if (result != cgltf_result_success){
			loge("cgltf couldn't load buffers : %i",result);
			exit(1);
		}

		size_t meshes_count = data->meshes_count;
		logd("Found %i meshes in model",data->meshes_count);
		cgltf_mesh* meshes = data->meshes;
		for(int i=0; i< meshes_count; i++){
			load_primitives_env(meshes,primitives,primitive_index);
			primitive_index += meshes->primitives_count;
		}

	}else{
		loge("Please check if model exists!");
		exit(1);
	}
	cgltf_free(data);
	return primitive_index;
 }

drawable_prim upload_single_primitive_env(primitive_env p){
	unsigned int ebo,vbo;
	drawable_prim m = {0};
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

void draw_single_primitive_env(unsigned int shaderProgram,drawable_prim d){
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

buffer append_primitive_vertice_data(primitive_env* primitives,size_t from,size_t to){
	vertex_env* vertices;
	size_t vertice_count =0;
	for(int i = from; i < to; i++){
		primitives[i].base_vertex = vertice_count;
		vertice_count += primitives[i].vertex_count;
	}
	logd("Total vertice count for environment mesh: %li",vertice_count);
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
	logd("Total indice count for environment mesh: %li",indices_count);
	indices = malloc(sizeof(unsigned short)*indices_count);
	unsigned short* ptr = indices;
	for(int i = from; i < to; i++){
		memcpy(ptr,primitives[i].indices,primitives[i].indices_count*sizeof(unsigned short));
		ptr += primitives[i].indices_count;
	}
	buffer buf = {indices,indices_count*sizeof(unsigned short)};
	return buf;
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
	logd("Size of vertices buffer: %li",vertices.size);
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

