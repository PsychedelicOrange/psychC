#include "../src/disk.h"
#include "../src/cJSON.h"
#include "../src/log.h"
#include <stdio.h>
#include <cglm/cglm.h> 
#include <cglm/struct.h>

/*typedef struct scene {*/
/*	char* version;*/
/*	object* objects;*/
/*	size_t objects_count;*/
/*	light* lights;*/
/*	size_t lights_count;*/
/*}scene;*/

typedef struct object{
	char* name;
	char* model_path;
	float scale[3];
	vec4s rotate[3];
	float translate[3];
	mat4s transform;
}object;

int main(){
	char* scene = read_string_from_disk("scene.json");
	const cJSON *json = cJSON_Parse(scene);
	{
		cJSON* objects = cJSON_GetObjectItemCaseSensitive(json, "objects");
		if(!cJSON_IsArray(objects)){ loge("value for key `objects` is not an array"); }
		else{
			cJSON* object = NULL;
			cJSON_ArrayForEach(object, objects){
				cJSON* name = cJSON_GetObjectItemCaseSensitive(object,"name");
				if( !cJSON_IsString(name) || name->valuestring == NULL){loge("value for key `name` is not a string");}
				else{
					logi("Name of object: %s",name->valuestring);
				}
			}
		}
	}
	char *string = cJSON_Print(json);
	logd("Read json: %s",string);
}
