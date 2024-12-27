/*
 * OpenAL Helpers
 *
 * Copyright (c) 2011 by Chris Robinson <chris.kcat@gmail.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

/*
 * The code is modified a bit from the original. - psyorange
 *
 */

#include "alhelper.h"

#include "log.h"
#include <errno.h>
#include <string.h>

#include "AL/al.h"
#include "AL/alc.h"
#include "AL/alext.h"
int al_init()
{
    const ALCchar *name;
    ALCdevice *device;
    ALCcontext *ctx;

	ALboolean enumeration;

	enumeration = alcIsExtensionPresent(NULL, "ALC_ENUMERATION_EXT");
	if (enumeration == AL_FALSE)
		logi("enumeration not supported");
	else
		logi("enumeration supported");
    /* Open and initialize a device */
	const char * devicename = alcGetString(NULL, ALC_DEFAULT_DEVICE_SPECIFIER);
	device  = alcOpenDevice(devicename);
    if(!device)
    {
		loge("Could'nt open device");
        return 1;
    }

    ctx = alcCreateContext(device, NULL);
    if(ctx == NULL || alcMakeContextCurrent(ctx) == ALC_FALSE)
    {
        if(ctx != NULL)
            alcDestroyContext(ctx);
        alcCloseDevice(device);
		loge("Couldn't set context");
        return 1;
    }

    name = NULL;
    if(alcIsExtensionPresent(device, "ALC_ENUMERATE_ALL_EXT"))
        name = alcGetString(device, ALC_ALL_DEVICES_SPECIFIER);
    if(!name || alcGetError(device) != AL_NO_ERROR)
        name = alcGetString(device, ALC_DEVICE_SPECIFIER);
    logi("Opened \"%s\"", name);

    return 0;
}

void al_close(void)
{
    ALCdevice *device;
    ALCcontext *ctx;

    ctx = alcGetCurrentContext();
    if(ctx == NULL)
        return;

    device = alcGetContextsDevice(ctx);

    alcMakeContextCurrent(NULL);
    alcDestroyContext(ctx);
    alcCloseDevice(device);
}

void al_list_devices() {
    // Get the list of devices
    const ALCchar *devices = alcGetString(NULL, ALC_DEVICE_SPECIFIER);
    if (!devices) {
        loge("Failed to get device list");
        return;
    }

    logi("Available audio devices:");

    // Iterate through the list
    const ALCchar *device = devices;
    while (*device != '\0') {
        logp(" - %s\n", device);
        device += strlen(device) + 1;  // Move to the next device string
    }

}

const char* al_format_name(ALenum format)
{
    switch(format)
    {
    case AL_FORMAT_MONO8: return "Mono, U8";
    case AL_FORMAT_MONO16: return "Mono, S16";
    case AL_FORMAT_MONO_FLOAT32: return "Mono, Float32";
    case AL_FORMAT_MONO_MULAW: return "Mono, muLaw";
    case AL_FORMAT_MONO_ALAW_EXT: return "Mono, aLaw";
    case AL_FORMAT_MONO_IMA4: return "Mono, IMA4 ADPCM";
    case AL_FORMAT_MONO_MSADPCM_SOFT: return "Mono, MS ADPCM";
    case AL_FORMAT_STEREO8: return "Stereo, U8";
    case AL_FORMAT_STEREO16: return "Stereo, S16";
    case AL_FORMAT_STEREO_FLOAT32: return "Stereo, Float32";
    case AL_FORMAT_STEREO_MULAW: return "Stereo, muLaw";
    case AL_FORMAT_STEREO_ALAW_EXT: return "Stereo, aLaw";
    case AL_FORMAT_STEREO_IMA4: return "Stereo, IMA4 ADPCM";
    case AL_FORMAT_STEREO_MSADPCM_SOFT: return "Stereo, MS ADPCM";
    case AL_FORMAT_QUAD8: return "Quadraphonic, U8";
    case AL_FORMAT_QUAD16: return "Quadraphonic, S16";
    case AL_FORMAT_QUAD32: return "Quadraphonic, Float32";
    case AL_FORMAT_QUAD_MULAW: return "Quadraphonic, muLaw";
    case AL_FORMAT_51CHN8: return "5.1 Surround, U8";
    case AL_FORMAT_51CHN16: return "5.1 Surround, S16";
    case AL_FORMAT_51CHN32: return "5.1 Surround, Float32";
    case AL_FORMAT_51CHN_MULAW: return "5.1 Surround, muLaw";
    case AL_FORMAT_61CHN8: return "6.1 Surround, U8";
    case AL_FORMAT_61CHN16: return "6.1 Surround, S16";
    case AL_FORMAT_61CHN32: return "6.1 Surround, Float32";
    case AL_FORMAT_61CHN_MULAW: return "6.1 Surround, muLaw";
    case AL_FORMAT_71CHN8: return "7.1 Surround, U8";
    case AL_FORMAT_71CHN16: return "7.1 Surround, S16";
    case AL_FORMAT_71CHN32: return "7.1 Surround, Float32";
    case AL_FORMAT_71CHN_MULAW: return "7.1 Surround, muLaw";
    case AL_FORMAT_BFORMAT2D_8: return "B-Format 2D, U8";
    case AL_FORMAT_BFORMAT2D_16: return "B-Format 2D, S16";
    case AL_FORMAT_BFORMAT2D_FLOAT32: return "B-Format 2D, Float32";
    case AL_FORMAT_BFORMAT2D_MULAW: return "B-Format 2D, muLaw";
    case AL_FORMAT_BFORMAT3D_8: return "B-Format 3D, U8";
    case AL_FORMAT_BFORMAT3D_16: return "B-Format 3D, S16";
    case AL_FORMAT_BFORMAT3D_FLOAT32: return "B-Format 3D, Float32";
    case AL_FORMAT_BFORMAT3D_MULAW: return "B-Format 3D, muLaw";
    case AL_FORMAT_UHJ2CHN8_SOFT: return "UHJ 2-channel, U8";
    case AL_FORMAT_UHJ2CHN16_SOFT: return "UHJ 2-channel, S16";
    case AL_FORMAT_UHJ2CHN_FLOAT32_SOFT: return "UHJ 2-channel, Float32";
    case AL_FORMAT_UHJ3CHN8_SOFT: return "UHJ 3-channel, U8";
    case AL_FORMAT_UHJ3CHN16_SOFT: return "UHJ 3-channel, S16";
    case AL_FORMAT_UHJ3CHN_FLOAT32_SOFT: return "UHJ 3-channel, Float32";
    case AL_FORMAT_UHJ4CHN8_SOFT: return "UHJ 4-channel, U8";
    case AL_FORMAT_UHJ4CHN16_SOFT: return "UHJ 4-channel, S16";
    case AL_FORMAT_UHJ4CHN_FLOAT32_SOFT: return "UHJ 4-channel, Float32";
    }
    return "Unknown Format";
}

