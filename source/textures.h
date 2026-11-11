#ifndef TEXTURES_H
#define TEXTURES_H

#include <nds.h>

#define TEX_ATLAS_W 256
#define TEX_ATLAS_H 16

void textures_init(void);
int  textures_get_id(void);

#endif // TEXTURES_H
