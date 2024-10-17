#ifndef CAMERA_H
#define CAMERA_H

#include <cglm/cglm.h>   /* for inline */
#include <cglm/struct.h>

typedef struct camera{
	mat4s lookAt;
	vec3s position;
	vec3s right;
	vec3s front;
	vec3s up;
	float yaw;
	float pitch;
}camera;

camera init_camera();

void update_camera_mouse_callback(camera* camera, float xoffset,float yoffset);

void update_third_person_camera(camera* camera,vec3s position);

void update_first_person_camera(camera* camera);

#endif
