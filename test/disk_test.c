#include <stdio.h>
#include <../src/disk.h>

int main(){
	char* string = read_string_from_disk("load_primitive_env.c");
	printf("Read %s",string);
}
