#include "cgltfhelper.h"
#include <stdlib.h>
#include <string.h>
#include "log.h"
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
