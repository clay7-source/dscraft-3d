#ifndef WORLD_H
#define WORLD_H

#include <nds.h>

#define WORLD_SIZE_X 32
#define WORLD_SIZE_Y 20
#define WORLD_SIZE_Z 32

typedef enum {
    BLOCK_AIR = 0,
    BLOCK_GRASS,
    BLOCK_DIRT,
    BLOCK_STONE,
    BLOCK_WOOD,
    BLOCK_LEAVES,
    BLOCK_PLANKS,
    BLOCK_COBBLE,
    BLOCK_BEDROCK,
    NUM_BLOCK_TYPES
} BlockType;

// Raycast result structure for mining & placing
typedef struct {
    bool hit;
    int hitX, hitY, hitZ;       // Block hit (for mining)
    int placeX, placeY, placeZ; // Adjacent block (for placing)
    BlockType blockType;
} RaycastHit;

void world_init(void);
BlockType world_get_block(int x, int y, int z);
void world_set_block(int x, int y, int z, BlockType type);
bool world_is_solid(int x, int y, int z);
void world_render(float camX, float camY, float camZ);
RaycastHit world_raycast(float originX, float originY, float originZ,
                         float dirX, float dirY, float dirZ, float maxDist);

#endif // WORLD_H
