#include "camera.h"


camera init_camera(){
	camera cam;
	cam.position.x = 0;
	cam.position.y = 1;
	cam.position.z = 5;

	cam.front.x = 0;
	cam.front.y = 0;
	cam.front.z = -1;

	cam.right.x = 1;
	cam.right.y = 0;
	cam.right.z = 0;
	cam.yaw = -90.0f;
	cam.pitch = 0;
	return cam;
}
void update_camera_mouse_callback(camera* camera, float xoffset,float yoffset){
	camera->yaw += xoffset;
	camera->pitch -= yoffset;
	if (camera->pitch > 89.0f)
		camera->pitch = 89.0f;
//	if (camera->pitch < 0)
//		camera->pitch = 0;
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


