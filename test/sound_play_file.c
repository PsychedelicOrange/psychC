#define AL_LIBTYPE_STATIC
#include <AL/alext.h>
#include <AL/al.h>
#include <AL/alc.h>
#include "alhelper.h"
#include "wave.h"
#include "log.h"
#include <stdlib.h>

ALenum getOpenALFormatEnum(fmt_chunk fmt){
	ALenum format;
    if(fmt.num_channels == 1 && fmt.bits_per_sample == 8)
        format = AL_FORMAT_MONO8;
    else if(fmt.num_channels == 1 && fmt.bits_per_sample == 16)
        format = AL_FORMAT_MONO16;
    else if(fmt.num_channels == 2 && fmt.bits_per_sample == 8)
        format = AL_FORMAT_STEREO8;
    else if(fmt.num_channels == 2 && fmt.bits_per_sample == 16)
        format = AL_FORMAT_STEREO16;
    else
	{
		loge(" wav format not supported : no. of channells = %i, \
				bits per sample = %i",fmt.num_channels,fmt.bits_per_sample); 
    }
	return format;
}

ALuint load_wav_into_buffer(wav_file wav){
	ALuint buffer;
	// clear error array
    ALenum error; alGetError();
	alGenBuffers(1,&buffer);
	if ((error = alGetError()) != AL_NO_ERROR || !buffer)
	{
		loge(" Couldn't generate buffer : %x", error);
		exit(1);
	}
	alBufferData(buffer,wav.format,wav.data,wav.size,wav.fmt.sample_rate);
	return buffer;
}

ALuint create_source(){
    ALuint source;
	// clear error array
    ALenum error; alGetError();
    alGenSources(1,&source);
    if ((error = alGetError()) != AL_NO_ERROR || !source)
    {
		loge("Couldn't generate source : %i", error);
		exit(1);
    }
	/*alSourcef(source, AL_GAIN, 1.0f);*/
	/*alSource3f(source, AL_POSITION, 0, 0, 0);*/
	/*alSource3f(source, AL_VELOCITY, 0, 0, 0);*/
	/*alSource3f(source, AL_DIRECTION,0,0,0);*/
	/*alSourcei(source, AL_LOOPING, AL_FALSE);*/

	return source;
}

int main(int argc, char *argv[]){
	wav_file wav = load_wav(argv[1]);
	{
		wav.format = getOpenALFormatEnum(wav.fmt);
		logi("Format : %s", al_format_name(wav.format));
	}
	al_init();
	al_list_devices();
	ALuint buffer = load_wav_into_buffer(wav);
	ALuint source = create_source();
    ALenum error = alGetError();
	alSourcei(source, AL_BUFFER, buffer);
	if ((error = alGetError()) != AL_NO_ERROR)
	{
		loge("Couldn't set buffer to source: %x", error);
		exit(1);
	}
	alSourcePlay(source);
	ALint state = AL_PLAYING;
	while(state == AL_PLAYING)
	{
		alGetSourcei( source, AL_SOURCE_STATE, &state);
	}

	al_close();
}
