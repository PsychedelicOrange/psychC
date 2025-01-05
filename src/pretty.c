#include "log.h"
void print_vec3(float* vec){
    logd("(%f,%f,%f)",vec[0],vec[1],vec[2]);
}
void print_vec4(float* vec){
    logd("(%f,%f,%f,%f)",vec[0],vec[1],vec[2],vec[3]);
}
void print_mat4_ptr(float* mat){
    logd("[");
    for(int i =0; i < 4; i++){
	for(int j =0; j < 4; j++){
	    logd("%f,",mat[(4*i)+j]);
	}
	logd("");
    }
    logd("]");
}
void print_mat4(float mat[4][4]){
    logd("[");
    for(int i =0; i < 4; i++){
	for(int j =0; j < 4; j++){
	    logd("%f,",mat[i][j]);
	}
	logd("");
    }
    logd("]");
}
