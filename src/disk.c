#include <stdio.h>
#include "constants.h"
void read_string_from_disk(const char* path, char* data){
	FILE* file = fopen(path, "r");
	if ( file == NULL ) {
		printf("Couln't open file: %s",path);
	}
	int err = fread(data,1,FILE_SIZE_SHADER_MAX,file);
	if ( err < 0 )
	{
		printf("Could'nt read file: %s",path);
	}
	fclose(file);
}
