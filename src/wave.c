#include "wave.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "log.h"

const char riff_id[4] = {'R','I','F','F'};
const char format[4] = {'W','A','V','E'};
const char fmt_id[4] = {'f','m','t',' '};
const char data_id[4] = {'d','a','t','a'};
const char meta_id[4] = {'I','N','F','O'};
const char list_id[4] = {'L','I','S','T'};

void* malloc_or_exit(size_t size_in_bytes){
	void* ptr = malloc(size_in_bytes);
	if(!ptr){
		loge("Couldn't malloc!"); 
		exit(1);
	}
	return ptr;
}
void fread_or_exit(void* dest, size_t size, size_t nitems, FILE* file){
	if(fread(dest,size,nitems,file) != nitems){
		loge("fread failed"); 
		exit(1);
	}
}
void parseHeader(FILE* file){
	char header_chunk_id[5];
	uint32_t header_chunk_size;
	char header_format[5];

	fread_or_exit(header_chunk_id,1,4,file) ;
	header_chunk_id[4] = '\0';
	fread_or_exit(&header_chunk_size,4,1,file) ;
	fread_or_exit(header_format,1,4,file) ;
	header_format[4] = '\0';
	logd("Header chunk id : %s",header_chunk_id); 
	logd("Header chunk size : %u",header_chunk_size); 
	logd("Header format : %s",header_format); 
	if(memcmp(header_chunk_id,riff_id,4) || memcmp(format,format,4)){
		loge("Header validation failed: incorrect format string or chunk_id in header"); 
		exit(1);
	}
	logi("Header validation succeeded"); 
}

wav_file load_wav(const char* file_path){
	logi("Loading wav file : %s",file_path); 
	wav_file wav;

	FILE* file = fopen(file_path,"r");
	if(!file){
		loge("Couldn't open file: %s",file_path); 
		exit(1);
	}

	parseHeader(file);

	fmt_chunk format_chunk;

	char chunk_id[4];
	while(fread(chunk_id, sizeof(char), 4, file) == 4 && memcmp(fmt_id,chunk_id,4)){
	}
	fseek(file,4,SEEK_CUR);
	fread_or_exit(&format_chunk.audio_format, sizeof(uint16_t), 1, file);
	fread_or_exit(&format_chunk.num_channels, sizeof(uint16_t), 1, file);
	fread_or_exit(&format_chunk.sample_rate, sizeof(uint32_t), 1, file);
	fread_or_exit(&format_chunk.byte_rate, sizeof(uint32_t), 1, file);
	fread_or_exit(&format_chunk.block_align, sizeof(uint16_t), 1, file);
	fread_or_exit(&format_chunk.bits_per_sample, sizeof(uint16_t), 1, file);
	wav.fmt = format_chunk;

	logd("format_chunk.audio_format : %u",format_chunk.audio_format);
	logd("format_chunk.num_channels : %u",format_chunk.num_channels);
	logd("format_chunk.sample_rate : %u",format_chunk.sample_rate);
	logd("format_chunk.byte_rate : %u",format_chunk.byte_rate);
	logd("format_chunk.block_align : %u",format_chunk.block_align);
	logd("format_chunk.bits_per_sample : %u",format_chunk.bits_per_sample);

	while(fread(chunk_id, sizeof(char), 4, file) == 4 && memcmp(data_id,chunk_id,4)){
	}
	fread_or_exit(&(wav.size), sizeof(uint32_t), 1, file);
	logd("Size of data : %u",wav.size);
	wav.data = malloc(wav.size);
	fread_or_exit(wav.data, 1, wav.size, file);
	logi("wav data read successfully");
	if (fclose(file) == EOF) {
       loge("failed to close file stream!");
	   exit(1);
   }

	return wav;
}
