#include "env.h"
#include "log.h"
#include <glad/glad.h>
#include <assert.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "cgltfhelper.h"
#include "stb_image.h"
#include "shader.h"

#define MAX_PATH_LEN 256
 
void bind_textures(material_env mat){
    if(mat.base_color != -1){
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D,mat.base_color);
    }
    if(mat.normal != -1){
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D,mat.normal);
    }
    if(mat.rough_metal != -1){
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D,mat.rough_metal);
    }
    if(mat.lightmap != -1){
	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_2D,mat.lightmap);
    }
}


unsigned int load_texture(cgltf_texture* texture){
    cgltf_buffer_view *buf_view = texture->image->buffer_view;

    int width, height, nrChannels;
    unsigned char *data;

    if(buf_view == NULL){ // image is external file
	logd("image uri: %s",texture->image->uri);
	char fullPath[MAX_PATH_LEN];
	sprintf(fullPath,"models/sponzaa/%s",texture->image->uri);
	logd("modified uri to path relative to executable, %s",fullPath);
	data = stbi_load(fullPath, &width, &height, &nrChannels, 0);
	if (!data){
	    loge("Couldn't load image : %s",fullPath);
	    exit(1);
	}
    }else{ // load from memory
	buffer image;
	image.data = (void *)((char *)buf_view->buffer->data + buf_view->offset);
	image.size = buf_view->size;
	//stbi_set_flip_vertically_on_load(1);
	data = stbi_load_from_memory(image.data,image.size, &width, &height, &nrChannels, 0); 
	if (!data){
	    loge("Couldn't load image from memory");
	    exit(1);
	}
    }

    int internal_format;
    switch(nrChannels){
	case 1:
	    internal_format = GL_RED;
	case 3:
	    internal_format = GL_RGB;
	    break;
	case 4:
	    internal_format = GL_RGBA;
	    break;
	default:
	    internal_format = GL_RGB;
	    logw("Not supported : image has %i no. of channels. Setting format to GL_RGB.",nrChannels);
    }

    unsigned int tex;
    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_2D, tex);

    { // set sampling properties
	cgltf_sampler* sampler = texture->sampler;
	if(sampler == NULL){
	    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	}else{
	    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, sampler->wrap_s);	
	    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, sampler->wrap_t);
	    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, sampler->min_filter);
	    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, sampler->mag_filter);
	}
    }

    glTexImage2D(GL_TEXTURE_2D, 0, internal_format
	    , width, height, 0, internal_format, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);

    stbi_image_free(data);
    return tex;
}

material_env load_material_env(cgltf_material* material){
    // load materials
    material_env mat;
    mat.base_color = -1;
    mat.rough_metal = -1;
    mat.normal = -1;
    /* TODO: Add light map */
    //mat.lightmap = lightmap;

    // textures
    if(material->has_pbr_metallic_roughness){
	if(material->pbr_metallic_roughness.base_color_texture.texture != NULL){
	    mat.base_color = load_texture(material->pbr_metallic_roughness.base_color_texture.texture);
	}else{
	    logw("material doesn't have base_color texture");
	}
	if(material->pbr_metallic_roughness.metallic_roughness_texture.texture != NULL){
	    mat.rough_metal = load_texture(material->pbr_metallic_roughness.metallic_roughness_texture.texture);
	}else{
	    logw("material doesn't have metallic_roughness texture");
	}
	mat.roughness_factor = material->pbr_metallic_roughness.roughness_factor;
	mat.metallic_factor = material->pbr_metallic_roughness.metallic_factor;
	memcpy(mat.base_color_factor , material->pbr_metallic_roughness.base_color_factor,sizeof(float) * 4);
	logd("Values for factors: base_color: [%f,%f,%f,%f] metal: %f, rough: %f", 
		mat.base_color_factor[0], mat.base_color_factor[1], mat.base_color_factor[2], mat.base_color_factor[3],
		mat.metallic_factor, mat.roughness_factor
	);
    }else{
	logw("material doesn't have pbr material");
    }

    if(material->normal_texture.texture != NULL){
	mat.normal = load_texture(material->normal_texture.texture);
    }
    return mat;
}

void upload_material_to_shader(unsigned int shader, material_env mat){
    setUniformInt(shader,
	    mat.base_color != -1,
	    "base_color_texture_exists");   
    setUniformInt(shader,
	    mat.rough_metal != -1,
	    "roughmetal_texture_exists");   
    setUniformInt(shader,
	    mat.normal != -1,
	    "normal_texture_exists");   
    setUniformFloat4(shader,
	    mat.base_color_factor,
	    "base_color_factor");   
    setUniformFloat(shader,
	    mat.metallic_factor,
	    "metallic_factor");   
    setUniformFloat(shader,
	    mat.roughness_factor,
	    "roughness_factor");   
}

void load_primitives_env (cgltf_mesh* mesh, primitive_env* primitives, size_t primitive_index){
    size_t mpriv_count = mesh->primitives_count;
    for(size_t i = 0; i < mpriv_count;i++){
	primitive_env p;
	size_t attribute_count = mesh->primitives[i].attributes_count;
	cgltf_attribute* attributes = mesh->primitives[i].attributes;
	fflush(stdout);
	p.vertices = malloc(sizeof(vertex_env)*attributes[0].data->count);
	p.vertex_count = attributes[0].data->count;

	p.material = load_material_env(mesh->primitives[i].material);

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
		    //print_accessor(accessor);
		    for(size_t k = 0; k < p.vertex_count; k++){
			memcpy(p.vertices[k].uv,att_buf,sizeof(float)*2);
			att_buf = (void*) ((char*)att_buf + accessor->stride);
		    }
		    break;
		default:
		    logw("Environment mesh doesn't support : %s",attributes[j].name);
	    }
	    fflush(stdout);	
	}
	primitives[primitive_index++] = p;
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
	    logd("loading mesh %i: %s",i,meshes[i].name);
	    load_primitives_env(&meshes[i],primitives,primitive_index);
	    primitive_index += meshes->primitives_count;
	}

    }else{
	loge("Please check if model exists!");
	exit(1);
    }
    cgltf_free(data);
    logd("Total primitive count: %i",primitive_index);
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
	//logd("glDrawElementsBaseVertex(GL_TRIANGLES, %li, GL_UNSIGNED_SHORT,(void*) %li, %li)" ,primitives[i].indices_count, (primitives[i].base_index) , primitives[i].base_vertex);
	bind_textures(primitives[i].material);
	upload_material_to_shader(shaderProgram, primitives[i].material);
	glDrawElementsBaseVertex(
		GL_TRIANGLES,
		primitives[i].indices_count,
		GL_UNSIGNED_SHORT,
		(void*)(primitives[i].base_index * sizeof(unsigned short)),
		primitives[i].base_vertex
		);
    }
}

buffer append_primitive_vertice_data(primitive_env* primitives,size_t from,size_t to){
    vertex_env* vertices;
    size_t vertice_count =0;
    for(int i = from; i < to; i++){
	/*logd("Primitive %i, vertex count -> %li",i,primitives[i].vertex_count);
	  logd("Primitive %i, vertex base count -> %li",i,vertice_count); */
	primitives[i].base_vertex = vertice_count;
	vertice_count += primitives[i].vertex_count;
    }
    logd("Total vertice count for environment mesh: %li",vertice_count);
    vertices = malloc(sizeof(vertex_env)*vertice_count);
    vertex_env* ptr = vertices;
    for(int i = from; i < to; i++){
	memcpy(ptr, primitives[i].vertices, primitives[i].vertex_count*sizeof(vertex_env));
	ptr += primitives[i].vertex_count;
    }
    buffer buf = {vertices,vertice_count*sizeof(vertex_env)};
    return buf;
}

buffer append_primitive_indice_data(primitive_env* primitives,size_t from,size_t to){
    unsigned short* indices;
    size_t indices_count = 0;
    for(int i = from; i < to; i++){
	primitives[i].base_index = indices_count;
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
    // smash primitives vertex data into a single buffer, 
    unsigned int vbo;
    unsigned int vao;
    unsigned int ebo;
    glGenVertexArrays(1,&vao);
    glGenBuffers(1, &vbo);
    glGenBuffers(1, &ebo);
    glBindVertexArray(vao); 
    buffer vertices = append_primitive_vertice_data(primitives,0,primitive_count);
    glBindBuffer(GL_ARRAY_BUFFER,vbo);
    glBufferData(GL_ARRAY_BUFFER, vertices.size,vertices.data,GL_STATIC_DRAW);
    free(vertices.data);

    buffer indices = append_primitive_indice_data(primitives,0,primitive_count);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size,indices.data,GL_STATIC_DRAW);
    free(indices.data);

    vertexAttrib_env();

    glBindBuffer(GL_ARRAY_BUFFER, 0); 
    glBindVertexArray(0); 

    return vao;
}
