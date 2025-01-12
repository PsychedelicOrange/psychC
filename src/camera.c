#include "camera.h"

camera init_camera(){
	camera cam;
	cam.position.x = 0;
	cam.position.y = 0;
	cam.position.z = 5;

	cam.front.x = 0;
	cam.front.y = 0;
	cam.front.z = -1;

	cam.up.x = 0;
	cam.up.y = 1;
	cam.up.z = 0;

	cam.right = glms_cross(cam.front,cam.up);
	cam.yaw = -90.;
	cam.pitch = 0;
	return cam;
}
void update_camera_mouse_callback(camera* camera, float xoffset,float yoffset){
	camera->yaw += xoffset;
	camera->pitch -= yoffset;
	if (camera->pitch > 89.0f)
		camera->pitch = 89.0f;
  	if (camera->pitch < -89.)
  		camera->pitch = -89.;
}


void update_third_person_camera(camera* camera, vec3s position){
	static vec3s up = {{0,1,0}};
	//cyaw = damper_exact(cyaw,yaw,1,delta_time);
	camera->position.x = cos(glm_rad(camera->pitch))*(sqrt(75))*cos(glm_rad(camera->yaw)) + position.x;
	camera->position.y = sin(glm_rad(camera->pitch))*(sqrt(75)) + position.y;
	camera->position.z = cos(glm_rad(camera->pitch))*(sqrt(75))*sin(glm_rad(camera->yaw)) + position.z;

	camera->front = glms_normalize(glms_vec3_sub(camera->position,position));
	camera->right = glms_normalize(glms_cross(camera->front, up));  // normalize the vectors, because their length gets closer to 0 the more you look up or down which results in slower movement.
	camera->up = glms_normalize(glms_cross(camera->right, camera->front));

	//printf("\r Camera.right: %f, %f, %f",camera.right.x,camera.right.y, camera.right.z);
}

void update_first_person_camera(camera* camera){
	static vec3s up = {{0,1,0}};
	vec3s front;
	front.x = cos(glm_rad(camera->yaw)) * cos(glm_rad(camera->pitch));
	front.y = sin(glm_rad(camera->pitch));
	front.z = sin(glm_rad(camera->yaw)) * cos(glm_rad(camera->pitch));
	camera->front = glms_normalize(front);
	camera->right = glms_normalize(glms_cross(camera->front, up));  // normalize the vectors, because their length gets closer to 0 the more you look up or down which results in slower movement.
	camera->up = glms_normalize(glms_cross(camera->right, camera->front));
}

void handle_camera_input(camera* cam, input i){
    float speed = 10 * i.delta_time;
    if( GLFW_PRESS == i.keys[GLFW_KEY_LEFT_SHIFT]){
	speed *= 0.1;
    }
    if( GLFW_PRESS ==  i.keys[GLFW_KEY_E]) {
	cam->position = glms_vec3_add(cam->position , glms_vec3_scale(cam->up,speed));
    }
    if( GLFW_PRESS == i.keys[GLFW_KEY_Q]){
	cam->position = glms_vec3_add(cam->position ,glms_vec3_scale(cam->up,-1 * speed));
    }
    if( GLFW_PRESS == i.keys[GLFW_KEY_W]){
	cam->position = glms_vec3_add(cam->position , glms_vec3_scale(cam->front,speed));
    }
    if( GLFW_PRESS == i.keys[GLFW_KEY_A]){
	cam->position = glms_vec3_add(cam->position ,glms_vec3_scale(cam->right,-1 * speed));
    }
    if( GLFW_PRESS == i.keys[GLFW_KEY_S]){
	cam->position = glms_vec3_add(cam->position ,glms_vec3_scale(cam->front,-1 * speed));
    }
    if( GLFW_PRESS == i.keys[GLFW_KEY_D]){
	cam->position = glms_vec3_add(cam->position ,glms_vec3_scale(cam->right, speed));
    }
}
