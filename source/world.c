#include "world.h"
#include <math.h>
#include <stdlib.h>

static u8 world_grid[WORLD_SIZE_X][WORLD_SIZE_Y][WORLD_SIZE_Z];

// Texture Atlas UV offsets (128x16 texture, each tile is 16x16)
// Tile indices: 0=GrassTop, 1=GrassSide, 2=Dirt, 3=Stone, 4=WoodSide, 5=WoodTop, 6=Leaves, 7=Planks
static inline int get_tile_u(BlockType type, int face) {
    // face: 0=Top, 1=Bottom, 2=Side
    switch (type) {
        case BLOCK_GRASS:
            if (face == 0) return 0;   // Grass top
            if (face == 1) return 32;  // Dirt bottom
            return 16;                 // Grass side
        case BLOCK_DIRT:
            return 32;
        case BLOCK_STONE:
        case BLOCK_COBBLE:
        case BLOCK_BEDROCK:
            return 48;
        case BLOCK_WOOD:
            if (face == 0 || face == 1) return 80; // Wood ring top
            return 64;                             // Wood bark side
        case BLOCK_LEAVES:
            return 96;
        case BLOCK_PLANKS:
            return 112;
        case BLOCK_SAND:
            return 128;
        case BLOCK_COAL_ORE:
            return 144;
        case BLOCK_IRON_ORE:
            return 160;
        case BLOCK_GLASS:
            return 176;
        case BLOCK_CRAFTING_TABLE:
            if (face == 0) return 192; // Grid top
            return 208;                // Tools side
        default:
            return 0;
    }
}

int world_get_surface_height(int x, int z) {
    if (x < 0) x = 0; if (x >= WORLD_SIZE_X) x = WORLD_SIZE_X - 1;
    if (z < 0) z = 0; if (z >= WORLD_SIZE_Z) z = WORLD_SIZE_Z - 1;
    for (int y = WORLD_SIZE_Y - 1; y >= 0; y--) {
        if (world_grid[x][y][z] != BLOCK_AIR) {
            return y;
        }
    }
    return 6;
}

static void spawn_tree(int tx, int ty, int tz) {
    if (tx < 2 || tx >= WORLD_SIZE_X - 2 || tz < 2 || tz >= WORLD_SIZE_Z - 2) return;
    if (ty + 5 >= WORLD_SIZE_Y) return;

    // Trunk
    for (int y = 0; y < 4; y++) {
        world_grid[tx][ty + y][tz] = BLOCK_WOOD;
    }

    // Leaves crown (3x3)
    int crownY = ty + 3;
    for (int dx = -1; dx <= 1; dx++) {
        for (int dz = -1; dz <= 1; dz++) {
            for (int dy = 0; dy <= 1; dy++) {
                int lx = tx + dx;
                int ly = crownY + dy;
                int lz = tz + dz;
                if (ly < WORLD_SIZE_Y) {
                    if (world_grid[lx][ly][lz] == BLOCK_AIR) {
                        world_grid[lx][ly][lz] = BLOCK_LEAVES;
                    }
                }
            }
        }
    }
    // Top leaf cap
    if (crownY + 2 < WORLD_SIZE_Y) {
        world_grid[tx][crownY + 2][tz] = BLOCK_LEAVES;
    }
}

void world_init(void) {
    // Procedural terrain generation
    for (int x = 0; x < WORLD_SIZE_X; x++) {
        for (int z = 0; z < WORLD_SIZE_Z; z++) {
            // Rolling hills height calculation
            float h1 = sinf(x * 0.25f) * 2.5f;
            float h2 = cosf(z * 0.30f) * 2.0f;
            int surfaceY = 7 + (int)(h1 + h2);
            if (surfaceY < 3) surfaceY = 3;
            if (surfaceY > WORLD_SIZE_Y - 7) surfaceY = WORLD_SIZE_Y - 7;

            for (int y = 0; y < WORLD_SIZE_Y; y++) {
                if (y == 0) {
                    world_grid[x][y][z] = BLOCK_BEDROCK;
                } else if (y < surfaceY - 3) {
                    if ((x * 7 + y * 13 + z * 11) % 17 == 0) {
                        world_grid[x][y][z] = BLOCK_COAL_ORE;
                    } else if ((x * 5 + y * 19 + z * 3) % 23 == 0) {
                        world_grid[x][y][z] = BLOCK_IRON_ORE;
                    } else {
                        world_grid[x][y][z] = BLOCK_STONE;
                    }
                } else if (y < surfaceY) {
                    world_grid[x][y][z] = (surfaceY <= 5) ? BLOCK_SAND : BLOCK_DIRT;
                } else if (y == surfaceY) {
                    world_grid[x][y][z] = (surfaceY <= 5) ? BLOCK_SAND : BLOCK_GRASS;
                } else {
                    world_grid[x][y][z] = BLOCK_AIR;
                }
            }

            // Scatter trees on grass
            if ((x % 6 == 2) && (z % 6 == 3)) {
                spawn_tree(x, surfaceY + 1, z);
            }
        }
    }
}

BlockType world_get_block(int x, int y, int z) {
    if (x < 0 || x >= WORLD_SIZE_X || y < 0 || y >= WORLD_SIZE_Y || z < 0 || z >= WORLD_SIZE_Z) {
        return BLOCK_AIR;
    }
    return (BlockType)world_grid[x][y][z];
}

void world_set_block(int x, int y, int z, BlockType type) {
    if (x >= 0 && x < WORLD_SIZE_X && y >= 0 && y < WORLD_SIZE_Y && z >= 0 && z < WORLD_SIZE_Z) {
        world_grid[x][y][z] = (u8)type;
    }
}

bool world_is_solid(int x, int y, int z) {
    BlockType b = world_get_block(x, y, z);
    return (b != BLOCK_AIR);
}

// Draw a single cube face with relative coordinates to camera and lighting
static void draw_face_top(float x, float y, float z, int u) {
    glColor3b(31, 31, 31); // Full sunlight on top
    int u0 = inttot16(u);
    int u1 = inttot16(u + 16);
    int v0 = inttot16(0);
    int v1 = inttot16(16);

    v16 x0 = floattov16(x),        x1 = floattov16(x + 1.0f);
    v16 y1 = floattov16(y + 1.0f);
    v16 z0 = floattov16(z),        z1 = floattov16(z + 1.0f);

    glTexCoord2t16(u0, v0); glVertex3v16(x0, y1, z0);
    glTexCoord2t16(u0, v1); glVertex3v16(x0, y1, z1);
    glTexCoord2t16(u1, v1); glVertex3v16(x1, y1, z1);
    glTexCoord2t16(u1, v0); glVertex3v16(x1, y1, z0);
}

static void draw_face_bottom(float x, float y, float z, int u) {
    glColor3b(16, 16, 16); // Shadow on bottom
    int u0 = inttot16(u);
    int u1 = inttot16(u + 16);
    int v0 = inttot16(0);
    int v1 = inttot16(16);

    v16 x0 = floattov16(x),        x1 = floattov16(x + 1.0f);
    v16 y0 = floattov16(y);
    v16 z0 = floattov16(z),        z1 = floattov16(z + 1.0f);

    glTexCoord2t16(u0, v0); glVertex3v16(x0, y0, z1);
    glTexCoord2t16(u0, v1); glVertex3v16(x0, y0, z0);
    glTexCoord2t16(u1, v1); glVertex3v16(x1, y0, z0);
    glTexCoord2t16(u1, v0); glVertex3v16(x1, y0, z1);
}

static void draw_face_north(float x, float y, float z, int u) { // +Z
    glColor3b(24, 24, 24);
    int u0 = inttot16(u);
    int u1 = inttot16(u + 16);
    int v0 = inttot16(0);
    int v1 = inttot16(16);

    v16 x0 = floattov16(x),        x1 = floattov16(x + 1.0f);
    v16 y0 = floattov16(y),        y1 = floattov16(y + 1.0f);
    v16 z1 = floattov16(z + 1.0f);

    glTexCoord2t16(u0, v1); glVertex3v16(x0, y0, z1);
    glTexCoord2t16(u1, v1); glVertex3v16(x1, y0, z1);
    glTexCoord2t16(u1, v0); glVertex3v16(x1, y1, z1);
    glTexCoord2t16(u0, v0); glVertex3v16(x0, y1, z1);
}

static void draw_face_south(float x, float y, float z, int u) { // -Z
    glColor3b(24, 24, 24);
    int u0 = inttot16(u);
    int u1 = inttot16(u + 16);
    int v0 = inttot16(0);
    int v1 = inttot16(16);

    v16 x0 = floattov16(x),        x1 = floattov16(x + 1.0f);
    v16 y0 = floattov16(y),        y1 = floattov16(y + 1.0f);
    v16 z0 = floattov16(z);

    glTexCoord2t16(u1, v1); glVertex3v16(x0, y0, z0);
    glTexCoord2t16(u1, v0); glVertex3v16(x0, y1, z0);
    glTexCoord2t16(u0, v0); glVertex3v16(x1, y1, z0);
    glTexCoord2t16(u0, v1); glVertex3v16(x1, y0, z0);
}

static void draw_face_east(float x, float y, float z, int u) { // +X
    glColor3b(28, 28, 28);
    int u0 = inttot16(u);
    int u1 = inttot16(u + 16);
    int v0 = inttot16(0);
    int v1 = inttot16(16);

    v16 x1 = floattov16(x + 1.0f);
    v16 y0 = floattov16(y),        y1 = floattov16(y + 1.0f);
    v16 z0 = floattov16(z),        z1 = floattov16(z + 1.0f);

    glTexCoord2t16(u0, v1); glVertex3v16(x1, y0, z0);
    glTexCoord2t16(u1, v1); glVertex3v16(x1, y0, z1);
    glTexCoord2t16(u1, v0); glVertex3v16(x1, y1, z1);
    glTexCoord2t16(u0, v0); glVertex3v16(x1, y1, z0);
}

static void draw_face_west(float x, float y, float z, int u) { // -X
    glColor3b(28, 28, 28);
    int u0 = inttot16(u);
    int u1 = inttot16(u + 16);
    int v0 = inttot16(0);
    int v1 = inttot16(16);

    v16 x0 = floattov16(x);
    v16 y0 = floattov16(y),        y1 = floattov16(y + 1.0f);
    v16 z0 = floattov16(z),        z1 = floattov16(z + 1.0f);

    glTexCoord2t16(u1, v1); glVertex3v16(x0, y0, z0);
    glTexCoord2t16(u1, v0); glVertex3v16(x0, y1, z0);
    glTexCoord2t16(u0, v0); glVertex3v16(x0, y1, z1);
    glTexCoord2t16(u0, v1); glVertex3v16(x0, y0, z1);
}

void world_render(float camX, float camY, float camZ) {
    int cx = (int)floorf(camX);
    int cy = (int)floorf(camY);
    int cz = (int)floorf(camZ);

    // View distance of 6 blocks ensures coordinates stay safely in v16 range [-7..+7]
    int viewDist = 6;
    int minX = cx - viewDist; if (minX < 0) minX = 0;
    int maxX = cx + viewDist; if (maxX >= WORLD_SIZE_X) maxX = WORLD_SIZE_X - 1;
    int minY = cy - viewDist; if (minY < 0) minY = 0;
    int maxY = cy + viewDist; if (maxY >= WORLD_SIZE_Y) maxY = WORLD_SIZE_Y - 1;
    int minZ = cz - viewDist; if (minZ < 0) minZ = 0;
    int maxZ = cz + viewDist; if (maxZ >= WORLD_SIZE_Z) maxZ = WORLD_SIZE_Z - 1;

    glBegin(GL_QUADS);

    for (int x = minX; x <= maxX; x++) {
        for (int z = minZ; z <= maxZ; z++) {
            for (int y = minY; y <= maxY; y++) {
                BlockType b = (BlockType)world_grid[x][y][z];
                if (b == BLOCK_AIR) continue;

                // Relative camera coordinates (strictly in range [-7..+7] for v16)
                float rx = (float)x - camX;
                float ry = (float)y - camY;
                float rz = (float)z - camZ;

                // Hidden face culling
                if (y + 1 >= WORLD_SIZE_Y || world_grid[x][y + 1][z] == BLOCK_AIR) {
                    draw_face_top(rx, ry, rz, get_tile_u(b, 0));
                }
                if (y > 0 && world_grid[x][y - 1][z] == BLOCK_AIR) {
                    draw_face_bottom(rx, ry, rz, get_tile_u(b, 1));
                }
                if (z + 1 >= WORLD_SIZE_Z || world_grid[x][y][z + 1] == BLOCK_AIR) {
                    draw_face_north(rx, ry, rz, get_tile_u(b, 2));
                }
                if (z - 1 < 0 || world_grid[x][y][z - 1] == BLOCK_AIR) {
                    draw_face_south(rx, ry, rz, get_tile_u(b, 2));
                }
                if (x + 1 >= WORLD_SIZE_X || world_grid[x + 1][y][z] == BLOCK_AIR) {
                    draw_face_east(rx, ry, rz, get_tile_u(b, 2));
                }
                if (x - 1 < 0 || world_grid[x - 1][y][z] == BLOCK_AIR) {
                    draw_face_west(rx, ry, rz, get_tile_u(b, 2));
                }
            }
        }
    }

    glEnd();
}

RaycastHit world_raycast(float ox, float oy, float oz,
                         float dx, float dy, float dz, float maxDist) {
    RaycastHit hit;
    hit.hit = false;

    float step = 0.08f;
    float dist = 0.0f;
    int lastX = (int)ox, lastY = (int)oy, lastZ = (int)oz;

    while (dist < maxDist) {
        float px = ox + dx * dist;
        float py = oy + dy * dist;
        float pz = oz + dz * dist;

        int bx = (int)floorf(px);
        int by = (int)floorf(py);
        int bz = (int)floorf(pz);

        if (bx >= 0 && bx < WORLD_SIZE_X &&
            by >= 0 && by < WORLD_SIZE_Y &&
            bz >= 0 && bz < WORLD_SIZE_Z) {
            BlockType b = (BlockType)world_grid[bx][by][bz];
            if (b != BLOCK_AIR) {
                hit.hit = true;
                hit.hitX = bx;
                hit.hitY = by;
                hit.hitZ = bz;
                hit.placeX = lastX;
                hit.placeY = lastY;
                hit.placeZ = lastZ;
                hit.blockType = b;
                return hit;
            }
        }

        lastX = bx;
        lastY = by;
        lastZ = bz;
        dist += step;
    }

    return hit;
}
