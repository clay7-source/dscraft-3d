#ifndef AUDIO_H
#define AUDIO_H

#include <nds.h>

typedef enum {
    SFX_STEP_GRASS = 0,
    SFX_STEP_STONE,
    SFX_DIG,
    SFX_PLACE,
    SFX_POP,
    SFX_CLICK,
    SFX_CRAFT,
    NUM_SFX
} SFXType;

void audio_init(void);
void audio_update(void);
void audio_play_sfx(SFXType type);
void audio_toggle_bgm(void);

#endif // AUDIO_H
