#include <AL/alext.h>
#include <AL/al.h>
#include <AL/alc.h>
#include "alhelper.h"
#include "log.h"
#include <stdlib.h>

int main(int argc, char *argv[]){
	al_init();
	al_list_devices();

	ALuint buffer = al_load(argv[1]);

	ALuint source = al_create_source();

	al_attach_buffer(source,buffer);

	alSourcePlay(source);

	ALint state = AL_PLAYING;
	while(state == AL_PLAYING)
	{
		alGetSourcei( source, AL_SOURCE_STATE, &state);
	}

	al_close();
}
