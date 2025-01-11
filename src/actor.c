#include "log.h"
#include "actor.h"
#include "glad/glad.h"
#include "GLFW/glfw3.h"
#include "pretty.h"

// private functions
void load_primitives_actor (cgltf_mesh* mesh, mesh_actor* emesh);
buffer calc_joint_topo_order(cgltf_data* data,cgltf_skin skin);
void topo_sort(cgltf_data* data, cgltf_node* joint, int* visited, int* vsize, int* topo_order, int* index);
void load_ibm(cgltf_skin* cskin ,skin* skin);
joint* load_joint(cgltf_data* data, cgltf_node* jnode);
void load_animations(cgltf_data* data,model_actor* model);
sampler load_sampler(cgltf_animation_sampler* gsamp);
buffer load_accessor(cgltf_accessor* acc);
size_t getBytesPerElement(cgltf_accessor* acc);
drawable_prim upload_single_primitive_actor(primitive_actor p);
void vertexAttrib_actor();
void animate(drawable_model* model);
int get_previous_index(float currentTime,buffer buffer);
void calc_global_transform_joint(hashmap_int* joints,skin* skin);
void calc_joint_matrices(float* matrices, skin* skin);

// prints
void print_indices(cgltf_accessor* indices){
    cgltf_buffer_view* buf_view = indices->buffer_view;
    logd("INDICES:");
    logd("indices.ctype:\t %i",indices->component_type);
    logd("indices.type:\t %i",indices->type);
    logd("indices.offset:\t%li",indices->offset);
    logd("indices.count:\t%li",indices->count);
    logd("indices.stride:\t%li",indices->stride);
    logd("buffer.name:\t%s",buf_view->buffer->name);
    logd("buffer_view.offset:\t%li",buf_view->offset);
    logd("buffer_view.size:\t%li",buf_view->size);
}
void print_accessor(cgltf_accessor *accessor ){
    cgltf_buffer_view* buf_view = accessor->buffer_view;
    logd("Accessor.ctype:\t %i",accessor->component_type);
    logd("Accessor.type:\t %i",accessor->type);
    logd("Accessor.offset:\t%li",accessor->offset);
    logd("Accessor.count:\t%li",accessor->count);
    logd("Accessor.stride:\t%li",accessor->stride);
    logd("buffer.name:\t%s",buf_view->buffer->name);
    logd("buffer_view.offset:\t%li",buf_view->offset);
    logd("buffer_view.size:\t%li",buf_view->size);
}
void print_vertex_actor(vertex_actor a){
    logd("float position[3]:");
    print_vec3(a.position);
    logd("float normal[3]:");
    print_vec3(a.normal);
    logd("float tangent[4]:");
    print_vec4(a.tangent);
    logd("unsigned short joints[4]:");
    logd("(%u,%u,%u,%u)",a.joints[0],a.joints[1],a.joints[2],a.joints[3]);
    logd("float weights[4]:");
    print_vec4(a.weights);
}

void print_sampler(sampler samp){
    logd("samp.keyframes: %li",samp.keyframes.size);
    logd("samp.data: %li",samp.data.size);
    logd("samp.element: %li",samp.element_size);
    logd("samp.interpolation: %i",samp.interpolation);
}
void print_joint(joint* j){
    logd("\tJoint \t :");
    logd("\t\tIndex \t %i:",j->gltf_index);
    logd("\t\tTranslation \t :");
    print_vec3(j->translation);
    logd("\t\tRotation \t :");
    print_vec4(j->rotation.raw);
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
	    loge("cgltf couldn't load buffers : %i",result);
	    exit(1);
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
			hashmap_insert(&model.joints,key,joint);
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
		// store joint rest translation and rotation in skin
		{
		    for(int k = 0; k < model.skins[i].joints_count;k++){
			memcpy(model.skins[i].joint_translation_rest[k],model.skins[i].joint_refs[k]->translation,sizeof(float)*3);
			memcpy(model.skins[i].joint_rotation_rest[k],model.skins[i].joint_refs[k]->rotation.raw,sizeof(float)*4);
		    }
		}
		// debug joints
		{
		    logd("joints:");
		    for(int j =0; j < model.joints.size;j++){
			logd("key: %i", model.joints.map[j].key);
			joint* joint = model.joints.map[j].value;
			print_joint(joint);
		    }
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
	loge("Please check if model exists!");
	exit(1);
    }
    cgltf_free(data);
    return model;
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
	    loge("encountered un-indexed mesh data. eww.");
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
		    logw("Actor mesh doesn't support : %s",attributes[j].name);
	    }
	}
	emesh->primitives[i] = p;
    }
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
    logd("Topological order for joint: ");
    for(int i = 0; i < jc;i ++){
	logd("%i, ",topo_order_ar[i]);
    }
    fflush(stdout);
    free(visited);
    return topo_order;
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

void load_animations(cgltf_data* data,model_actor* model){
    int anim_count = data->animations_count;
    assert(anim_count < 10);
    model->animations_count = anim_count;

    for(int i = 0; i < anim_count; i++){
	//printf("\n Animation found: %s",data->animations[i].name);
	animation animation;
	animation.started = 0;
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

size_t getBytesPerElement(cgltf_accessor* acc){
    cgltf_component_type ctype = acc->component_type;
    cgltf_type type = acc->type;
    int cgltf_ctype_to_bytes[] = {-1,1,1,2,2,4,4};
    int cgltf_type_to_count[]= {-1,1,2,3,4,4,9,16};	
    return cgltf_ctype_to_bytes[ctype]*type;
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

void draw_model(drawable_model* dmodel,unsigned int shaderProgram){
    for(int i = 0; i < dmodel->meshes_count; i++){
	// TODO combine vbos for all vbos in future ( check materials flow ?)
	drawable_mesh* mesh_actor = &dmodel->meshes[i];
	if(mesh_actor->skin_ref != NULL){
	    animate(dmodel);
	    calc_global_transform_joint(dmodel->joints,mesh_actor->skin_ref);
	    calc_joint_matrices(mesh_actor->jointMatricesData,mesh_actor->skin_ref);
	    int jointmat_loc = glGetUniformLocation(shaderProgram, "u_jointMat");
	    glUseProgram(shaderProgram);
	    glUniformMatrix4fv(jointmat_loc,mesh_actor->skin_ref->joints_count,GL_FALSE,mesh_actor->jointMatricesData);	
	}
    for (int p = 0; p < mesh_actor->prims_count; p++)
    {
        draw_single_primitive_actor(shaderProgram, mesh_actor->prims[p]);
    }
    }
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
			    float endTime = ((float *)samp.keyframes.data)[samp.keyframes.size/sizeof(float)-1];
			    if(currentTime > endTime){
				model->animations[j].started = -1;
				memcpy(channel->data_ptr,&(samp.data.data[0]),samp.element_size);
				break;
			    }else{
				memcpy(channel->data_ptr,&(samp.data.data[samp.element_size*previousTimeIndex]),samp.element_size);
			    }
			}
			break;
		    case LINEAR:
			{
			    float currentTime = glfwGetTime() - model->animations[j].started;
			    int previousTimeIndex = get_previous_index(currentTime,samp.keyframes);
			    float endTime = ((float *)samp.keyframes.data)[samp.keyframes.size/sizeof(float)-1];
			    if(currentTime > endTime){
				model->animations[j].started = -1;
				previousTimeIndex = (samp.keyframes.size/sizeof(float))-2;
				break;
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
	}else if (model->animations[j].started == -1 ){// set to rest
	    for(int i = 0; i < model->meshes_count; i++){
		skin* skin_ref = model->meshes[i].skin_ref;
		for(int j = 0; j < skin_ref->joints_count; j++){
		    // for each joint, restore translation and rotatation to rest
		    memcpy(skin_ref->joint_refs[j]->translation,skin_ref->joint_translation_rest[j],sizeof(float)*3);
		    memcpy(skin_ref->joint_refs[j]->rotation.raw,skin_ref->joint_rotation_rest[j],sizeof(float)*4);
		}
	    }
	    model->animations[j].started = 0;
	}else {
	    // don't animate further
	}
    }
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


void calc_global_transform_joint(hashmap_int* joints,skin* skin){
    size_t jc = skin->joints_count;

    // calc global transforms for each joint (in reverse topo order so its simple)
    int* order = ((int*)skin->topo_order.data);
    for (int i = (skin->topo_order.size/sizeof(int)) - 1; i >= 0 ; i--){
	joint* j = hashmap_get(joints,order[i]);
	if(!j->parent){
	    mat4 m;
	    glm_translate_make(m,j->translation);
	    glm_quat_rotate(m,j->rotation.raw,j->transform.raw);
	}else{
	    // globalTransformOfSelf = (globalTransformOfParent * localTransformOfSelf)
	    mat4 m;
	    glm_translate_make(m,j->translation);
	    mat4 rotated;
	    glm_quat_rotate(m,j->rotation.raw,rotated);
	    glm_mat4_mul(j->parent->transform.raw,rotated,j->transform.raw);
	}
    }

}

void calc_joint_matrices(float* matrices, skin* skin){
    size_t jc = skin->joints_count;
    float* ptr = matrices;
    for(int i =0; i < jc; i++){
	joint* j = skin->joint_refs[i];
	mat4s m = GLMS_MAT4_IDENTITY_INIT;
	glm_mat4_mul(j->transform.raw,skin->inverseBindMatrices[i].raw,m.raw);
	memcpy(ptr,m.raw,sizeof(float)*16);
	ptr += 16;
    }
}

void draw_single_primitive_actor(unsigned int shaderProgram,drawable_prim d){
	glUseProgram(shaderProgram);
	glBindVertexArray(d.vao);
	glDrawElements(GL_TRIANGLES, d.indices_count,GL_UNSIGNED_SHORT, 0);
}
