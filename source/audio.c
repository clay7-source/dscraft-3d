#include "audio.h"
#include <filesystem.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    s16 *data;
    u32 size;
} SoundSample;

static SoundSample sfx_samples[NUM_SFX];
static s16 *bgm_buffer = NULL;
static u32 bgm_buffer_size = 0;
static int bgm_channel = -1;
static bool nitro_mounted = false;

static const char *SFX_FILES[NUM_SFX] = {
    "nitro:/sfx/step_grass.raw",
    "nitro:/sfx/step_stone.raw",
    "nitro:/sfx/dig.raw",
    "nitro:/sfx/place.raw",
    "nitro:/sfx/pop.raw",
    "nitro:/sfx/click.raw",
    "nitro:/sfx/craft.raw"
};

// Fallback procedural SFX in case nitroFS is unmounted
static void generate_fallback_sfx(void) {
    for (int i = 0; i < NUM_SFX; i++) {
        if (!sfx_samples[i].data) {
            int len = 2000;
            s16 *buf = (s16*)malloc(len * sizeof(s16));
            if (buf) {
                for (int s = 0; s < len; s++) {
                    float t = (float)s / 22050.0f;
                    buf[s] = (s16)((rand() % 4000 - 2000) * (1.0f - t * 10.0f));
                }
                sfx_samples[i].data = buf;
                sfx_samples[i].size = len * sizeof(s16);
            }
        }
    }
}

void audio_init(void) {
    // Enable Nintendo DS sound hardware
    soundEnable();

    // Mount virtual NitroFS ROM filesystem
    nitro_mounted = nitroFSInit(NULL);

    if (nitro_mounted) {
        // 1. Load sound effects into RAM
        for (int i = 0; i < NUM_SFX; i++) {
            FILE *f = fopen(SFX_FILES[i], "rb");
            if (f) {
                fseek(f, 0, SEEK_END);
                long sz = ftell(f);
                fseek(f, 0, SEEK_SET);

                s16 *buf = (s16*)malloc(sz);
                if (buf) {
                    fread(buf, 1, sz, f);
                    sfx_samples[i].data = buf;
                    sfx_samples[i].size = (u32)sz;
                }
                fclose(f);
            }
        }

        // 2. Load ambient music loop into RAM (~1.2 MB for 28 seconds of piano)
        FILE *fMusic = fopen("nitro:/music/calm.raw", "rb");
        if (fMusic) {
            u32 maxBgmBytes = 28 * 22050 * 2; // 28 seconds @ 22kHz 16-bit
            s16 *buf = (s16*)malloc(maxBgmBytes);
            if (buf) {
                size_t readBytes = fread(buf, 1, maxBgmBytes, fMusic);
                bgm_buffer = buf;
                bgm_buffer_size = (u32)readBytes;

                // Start ambient piano music loop on channel 0
                bgm_channel = soundPlaySample(bgm_buffer, SoundFormat_16Bit,
                                              bgm_buffer_size, 22050,
                                              70, 64, true, 0);
            }
            fclose(fMusic);
        }
    }

    generate_fallback_sfx();
}

void audio_update(void) {
    // Nothing continuous needed; hardware handles looping
}

void audio_play_sfx(SFXType type) {
    if (type >= 0 && type < NUM_SFX && sfx_samples[type].data) {
        soundPlaySample(sfx_samples[type].data, SoundFormat_16Bit,
                        sfx_samples[type].size, 22050,
                        110, 64, false, 0);
    }
}

void audio_toggle_bgm(void) {
    if (bgm_channel >= 0) {
        soundKill(bgm_channel);
        bgm_channel = -1;
    } else if (bgm_buffer && bgm_buffer_size > 0) {
        bgm_channel = soundPlaySample(bgm_buffer, SoundFormat_16Bit,
                                      bgm_buffer_size, 22050,
                                      70, 64, true, 0);
    }
}
