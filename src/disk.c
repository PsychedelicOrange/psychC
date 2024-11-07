#include <stdio.h>
#include <stdlib.h>
char* read_string_from_disk(const char* filename){
	// Open the file in read mode ("r")
	FILE* file = fopen(filename, "r");
	if (!file) {
		printf("\nCould not open file %s\n", filename);
		fflush(stdout);
		return NULL;
	}

	// Move the file pointer to the end of the file to determine file size
	fseek(file, 0, SEEK_END);
	size_t fileSize = ftell(file);
	rewind(file); // Move file pointer back to the beginning

	// Allocate memory for the file content (+1 for the null terminator)
	char* content = (char*)malloc((fileSize + 1) * sizeof(char));
	if (!content) {
		printf("\nMemory allocation failed\n");
		fclose(file);
		return NULL;
	}
	for (size_t i = 0; i < fileSize + 1; i++) {
		content[i] = '\0';
	}
	// Read file contents into the string
	fread(content, sizeof(char), fileSize, file);

	// Close the file and return the content
	fclose(file);
	return content;
}
