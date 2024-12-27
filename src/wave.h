#ifndef WAVE_H
#define WAVE_H

#include <stddef.h>
#include <stdint.h>

typedef struct fmt_chunk{
    uint16_t audio_format;
    uint16_t num_channels;
    uint32_t sample_rate;
    uint32_t byte_rate;
    uint16_t block_align;
    uint16_t bits_per_sample;
}fmt_chunk;

typedef struct wav_file{
    void* data;
    uint32_t size;
    fmt_chunk fmt;
    int format; // OpenALenum 
}wav_file;

wav_file load_wav(char* path);

#endif
