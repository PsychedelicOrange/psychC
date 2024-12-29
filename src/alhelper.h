#ifndef ALHELPERS_H
#define ALHELPERS_H

#define AL_LIBTYPE_STATIC

#include "AL/alext.h"

/* init openal context */ 
void al_init();

void al_list_devices(); 

/* loads wav file from disk then loads data into openal buffer and returns buffer (sync) */
ALuint al_load(const char* file_path);

ALuint al_create_source();

void al_attach_buffer(ALuint source, ALuint buffer);

void al_close();

const char* al_format_name(ALenum format);

ALenum getOpenALFormatEnum(int num_channels, int bits_per_sample );

#endif 

