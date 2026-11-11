#include "textures.h"
#include <stdlib.h>

static int global_texture_id = 0;
static u16 texture_atlas[TEX_ATLAS_W * TEX_ATLAS_H];

// Helper to write pixel to atlas
static inline void set_pixel(int x, int y, u16 color) {
    if (x >= 0 && x < TEX_ATLAS_W && y >= 0 && y < TEX_ATLAS_H) {
        texture_atlas[y * TEX_ATLAS_W + x] = color | (1 << 15); // Alpha bit set
    }
}

void textures_init(void) {
    // Seed for reproducible texture speckles
    srand(1337);

    for (int y = 0; y < 16; y++) {
        for (int x = 0; x < 16; x++) {
            // 0: Grass Top (Light green with speckles)
            int gRand = rand() % 5;
            u16 grassColor = RGB15(6 + gRand / 2, 22 + gRand, 6 + gRand / 2);
            set_pixel(x + 0, y, grassColor);

            // 1: Grass Side (Green top 4 pixels, dirt brown bottom)
            if (y < 3 || (y == 3 && (x % 3 != 0))) {
                set_pixel(x + 16, y, grassColor);
            } else {
                int dRand = rand() % 4;
                set_pixel(x + 16, y, RGB15(14 + dRand, 9 + dRand, 4 + dRand / 2));
            }

            // 2: Dirt (Rich earthy brown)
            int dRand = rand() % 4;
            set_pixel(x + 32, y, RGB15(15 + dRand, 10 + dRand, 5 + dRand / 2));

            // 3: Stone (Gray with flecks)
            int sRand = rand() % 6;
            set_pixel(x + 48, y, RGB15(15 + sRand, 15 + sRand, 15 + sRand));

            // 4: Wood Trunk Side (Vertical oak bark stripes)
            int stripe = (x % 4 == 0) ? 2 : 0;
            int wRand = rand() % 3;
            set_pixel(x + 64, y, RGB15(14 - stripe + wRand, 9 - stripe + wRand, 4 + wRand));

            // 5: Wood Trunk Top (Tree rings)
            int distCenter = (x - 8) * (x - 8) + (y - 8) * (y - 8);
            if (x == 0 || x == 15 || y == 0 || y == 15) {
                set_pixel(x + 80, y, RGB15(12, 7, 3)); // Bark edge
            } else if (distCenter > 18 && distCenter < 28) {
                set_pixel(x + 80, y, RGB15(18, 13, 7)); // Ring
            } else {
                set_pixel(x + 80, y, RGB15(22, 17, 10)); // Inner wood
            }

            // 6: Leaves (Green canopy pattern)
            int lRand = rand() % 7;
            set_pixel(x + 96, y, RGB15(4 + lRand / 2, 18 + lRand, 5 + lRand / 3));

            // 7: Planks (Horizontal wooden oak boards)
            if (y % 4 == 0 || (y < 4 && x == 8) || (y >= 8 && y < 12 && x == 4)) {
                set_pixel(x + 112, y, RGB15(14, 10, 5)); // Plank seam
            } else {
                int pRand = rand() % 3;
                set_pixel(x + 112, y, RGB15(21 + pRand, 16 + pRand, 9 + pRand));
            }

            // 8: Sand (Golden beach sand)
            int sndRand = rand() % 3;
            set_pixel(x + 128, y, RGB15(28 + sndRand, 26 + sndRand, 16 + sndRand));

            // 9: Coal Ore (Stone with dark carbon deposits)
            int coalDep = (rand() % 6 == 0);
            set_pixel(x + 144, y, coalDep ? RGB15(2, 2, 2) : RGB15(15 + sRand, 15 + sRand, 15 + sRand));

            // 10: Iron Ore (Stone with rust deposits)
            int ironDep = (rand() % 6 == 0);
            set_pixel(x + 160, y, ironDep ? RGB15(25, 14, 8) : RGB15(15 + sRand, 15 + sRand, 15 + sRand));

            // 11: Glass (Clear border with specular lines)
            if (x == 0 || x == 15 || y == 0 || y == 15 || (x == y && x > 3 && x < 8)) {
                set_pixel(x + 176, y, RGB15(31, 31, 31));
            } else {
                set_pixel(x + 176, y, RGB15(8, 16, 22));
            }

            // 12: Crafting Table Top (3x3 grid over wood)
            if (x == 5 || x == 10 || y == 5 || y == 10) {
                set_pixel(x + 192, y, RGB15(12, 7, 3));
            } else {
                set_pixel(x + 192, y, RGB15(22, 17, 9));
            }

            // 13: Crafting Table Side (Oak wood with side tools)
            if (y > 4 && y < 12 && x > 4 && x < 12) {
                set_pixel(x + 208, y, RGB15(10, 8, 8)); // Tool slot
            } else {
                set_pixel(x + 208, y, RGB15(20, 15, 8));
            }
        }
    }

    // Load into DS 3D Texture VRAM
    glGenTextures(1, &global_texture_id);
    glBindTexture(0, global_texture_id);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, TEXTURE_SIZE_256, TEXTURE_SIZE_16, 0,
                 TEXGEN_TEXCOORD | GL_TEXTURE_WRAP_S | GL_TEXTURE_WRAP_T, (u8*)texture_atlas);
}

int textures_get_id(void) {
    return global_texture_id;
}
