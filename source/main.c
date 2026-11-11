#include <nds.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "world.h"
#include "camera.h"
#include "textures.h"
#include "inventory.h"
#include "ui.h"
#include "audio.h"

static void update_sky_color(u32 tick) {
    u32 phase = tick % 7200; // ~2 min day/night cycle at 60 FPS
    u8 r, g, b;

    if (phase < 3600) {
        // Day: Crisp light blue sky
        r = 12; g = 22; b = 31;
    } else if (phase < 4200) {
        // Sunset transition: warm golden amber to crimson
        float t = (float)(phase - 3600) / 600.0f;
        r = (u8)(12.0f * (1.0f - t) + 28.0f * t);
        g = (u8)(22.0f * (1.0f - t) + 12.0f * t);
        b = (u8)(31.0f * (1.0f - t) + 8.0f * t);
    } else if (phase < 6600) {
        // Night: deep starry midnight blue
        r = 2; g = 3; b = 6;
    } else {
        // Sunrise transition: rose dawn to daylight
        float t = (float)(phase - 6600) / 600.0f;
        r = (u8)(2.0f * (1.0f - t) + 12.0f * t);
        g = (u8)(3.0f * (1.0f - t) + 22.0f * t);
        b = (u8)(6.0f * (1.0f - t) + 31.0f * t);
    }

    glClearColor(r, g, b, 31);
}

static void render_first_person_hand(ItemType held, int swingTimer, float walkBob) {
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    float swing = (swingTimer > 0) ? sinf((10 - swingTimer) * 0.314f) * 0.035f : 0.0f;
    float bobY = sinf(walkBob) * 0.004f;
    float bobX = cosf(walkBob * 0.5f) * 0.004f;

    float bx = 0.13f + bobX;
    float by = -0.11f - swing + bobY;
    float bz = -0.28f - swing * 0.4f;

    BlockType bt = item_to_block(held);

    glPolyFmt(POLY_ALPHA(31) | POLY_CULL_NONE | POLY_ID(2));

    if (bt != BLOCK_AIR) {
        // Render miniature held block
        glBindTexture(0, textures_get_id());
        glColor3b(31, 31, 31);
        float s = 0.042f;
        v16 x0 = floattov16(bx),     x1 = floattov16(bx + s);
        v16 y0 = floattov16(by),     y1 = floattov16(by + s);
        v16 z0 = floattov16(bz),     z1 = floattov16(bz + s);

        int u = 0;
        switch (bt) {
            case BLOCK_DIRT:           u = 16; break;
            case BLOCK_WOOD:           u = 32; break;
            case BLOCK_LEAVES:         u = 48; break;
            case BLOCK_STONE:          u = 64; break;
            case BLOCK_PLANKS:         u = 80; break;
            case BLOCK_SAND:           u = 96; break;
            case BLOCK_COAL_ORE:       u = 112; break;
            case BLOCK_IRON_ORE:       u = 128; break;
            case BLOCK_GLASS:          u = 144; break;
            case BLOCK_CRAFTING_TABLE: u = 160; break;
            default:                   u = 16; break;
        }

        int u0 = inttot16(u), u1 = inttot16(u + 16);
        int v0 = inttot16(0), v1 = inttot16(16);

        glBegin(GL_QUADS);
            // Front (+Z)
            glTexCoord2t16(u0, v1); glVertex3v16(x0, y0, z1);
            glTexCoord2t16(u1, v1); glVertex3v16(x1, y0, z1);
            glTexCoord2t16(u1, v0); glVertex3v16(x1, y1, z1);
            glTexCoord2t16(u0, v0); glVertex3v16(x0, y1, z1);
            // Top (+Y)
            glTexCoord2t16(u0, v1); glVertex3v16(x0, y1, z1);
            glTexCoord2t16(u1, v1); glVertex3v16(x1, y1, z1);
            glTexCoord2t16(u1, v0); glVertex3v16(x1, y1, z0);
            glTexCoord2t16(u0, v0); glVertex3v16(x0, y1, z0);
            // Left (-X)
            glTexCoord2t16(u1, v1); glVertex3v16(x0, y0, z0);
            glTexCoord2t16(u0, v1); glVertex3v16(x0, y0, z1);
            glTexCoord2t16(u0, v0); glVertex3v16(x0, y1, z1);
            glTexCoord2t16(u1, v0); glVertex3v16(x0, y1, z0);
        glEnd();
    } else {
        // Steve player arm (classic flesh colored cuboid)
        glBindTexture(0, 0);
        glColor3b(24, 18, 14);
        float w = 0.035f, h = 0.075f, d = 0.05f;
        v16 x0 = floattov16(bx),     x1 = floattov16(bx + w);
        v16 y0 = floattov16(by),     y1 = floattov16(by + h);
        v16 z0 = floattov16(bz),     z1 = floattov16(bz + d);

        glBegin(GL_QUADS);
            // Top
            glVertex3v16(x0, y1, z1); glVertex3v16(x1, y1, z1);
            glVertex3v16(x1, y1, z0); glVertex3v16(x0, y1, z0);
            // Front
            glVertex3v16(x0, y0, z1); glVertex3v16(x1, y0, z1);
            glVertex3v16(x1, y1, z1); glVertex3v16(x0, y1, z1);
            // Left
            glVertex3v16(x0, y0, z0); glVertex3v16(x0, y0, z1);
            glVertex3v16(x0, y1, z1); glVertex3v16(x0, y1, z0);
        glEnd();
    }
}

int main(void) {
    // 1. Power on 2D and 3D graphics hardware
    powerOn(POWER_ALL_2D | POWER_3D_CORE | POWER_MATRIX);

    // 2. Setup Top Screen for 3D OpenGL Rendering
    videoSetMode(MODE_0_3D);
    vramSetBankA(VRAM_A_TEXTURE); // VRAM Bank A for 3D Textures

    glInit();
    glEnable(GL_TEXTURE_2D | GL_ANTIALIAS);

    glClearColor(12, 22, 31, 31);
    glClearPolyID(63);
    glClearDepth(0x7FFF);
    glViewport(0, 0, 255, 191);

    // 3D Perspective Projection
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(65.0f, 256.0f / 192.0f, 0.1f, 40.0f);

    // 3. Initialize Sub Screen 2D Framebuffer for Touch HUD
    ui_init();

    // 4. Initialize Game Subsystems
    world_init();
    textures_init();
    audio_init();

    // Spawn player safely atop the generated landscape
    int spawnSurface = world_get_surface_height(16, 16);
    float spawnY = (float)spawnSurface + 1.8f;

    Camera camera;
    camera_init(&camera, 16.5f, spawnY, 16.5f);

    Inventory inventory;
    inventory_init(&inventory);

    UIState ui;
    ui.mode = UI_MODE_GAME;
    ui.requestMine = false;
    ui.requestPlace = false;
    ui.requestJump = false;

    touchPosition lastTouch = {0, 0};
    bool wasTouching = false;

    u32 gameTick = 0;
    int swingTimer = 0;
    float stepAccumulator = 0.0f;
    float walkBob = 0.0f;

    // Main Game Loop (Locked at 60 FPS via VBlank)
    while (pmMainLoop()) {
        swiWaitForVBlank();

        scanKeys();
        u32 kHeld = keysHeld();
        u32 kDown = keysDown();

        touchPosition touch;
        touchRead(&touch);

        gameTick++;
        update_sky_color(gameTick);

        // --- Touchpad Stylus Mouse-Look Calculation ---
        int deltaTouchX = 0;
        int deltaTouchY = 0;

        if (kHeld & KEY_TOUCH) {
            if (ui.mode == UI_MODE_GAME) {
                // Only treat as camera drag if touching inside the touchpad region
                if (touch.px >= 50 && touch.px <= 206 && touch.py >= 28 && touch.py <= 145) {
                    if (wasTouching) {
                        deltaTouchX = touch.px - lastTouch.px;
                        deltaTouchY = touch.py - lastTouch.py;

                        // Deadzone filter for stylus jitter
                        if (abs(deltaTouchX) > 40) deltaTouchX = 0;
                        if (abs(deltaTouchY) > 40) deltaTouchY = 0;
                    }
                }
            }
            lastTouch = touch;
            wasTouching = true;
        } else {
            wasTouching = false;
        }

        // --- Update UI & Touch Buttons ---
        ui_update(&ui, &inventory, &touch, kDown, kHeld);

        // --- Update Camera & Movement ---
        camera_update(&camera, kHeld, kDown, deltaTouchX, deltaTouchY);
        if (ui.requestJump && camera.onGround) {
            camera.velY = 0.22f;
            camera.onGround = false;
        }

        // Footstep audio & walk bob
        bool isMoving = (kHeld & (KEY_UP | KEY_DOWN | KEY_LEFT | KEY_RIGHT)) != 0;
        if (isMoving && camera.onGround) {
            walkBob += 0.25f;
            stepAccumulator += 0.05f;
            if (stepAccumulator >= 0.5f) {
                stepAccumulator = 0.0f;
                audio_play_sfx(SFX_STEP_GRASS);
            }
        }

        // --- Raycasting for Aimed Block ---
        float dirX, dirY, dirZ;
        camera_get_direction(&camera, &dirX, &dirY, &dirZ);
        RaycastHit hit = world_raycast(camera.x, camera.y, camera.z,
                                       dirX, dirY, dirZ, 4.5f);

        // --- Mining Blocks ---
        if (ui.requestMine && hit.hit) {
            if (hit.blockType != BLOCK_BEDROCK && hit.blockType != BLOCK_AIR) {
                ItemType droppedItem = block_to_item(hit.blockType);
                inventory_add_item(&inventory, droppedItem, 1);
                world_set_block(hit.hitX, hit.hitY, hit.hitZ, BLOCK_AIR);
                audio_play_sfx(SFX_DIG);
                audio_play_sfx(SFX_POP);
                swingTimer = 10;
            }
        }

        // --- Placing Blocks ---
        if (ui.requestPlace && hit.hit) {
            ItemType selItem = inventory_get_selected_type(&inventory);
            BlockType blockToPlace = item_to_block(selItem);

            if (blockToPlace != BLOCK_AIR) {
                // Ensure player isn't standing directly inside placement spot
                int pBlockX = (int)floorf(camera.x);
                int pBlockY1 = (int)floorf(camera.y);
                int pBlockY2 = (int)floorf(camera.y - 1.0f);
                int pBlockZ = (int)floorf(camera.z);

                bool collidesWithPlayer = (hit.placeX == pBlockX && hit.placeZ == pBlockZ) &&
                                          (hit.placeY == pBlockY1 || hit.placeY == pBlockY2);

                if (!collidesWithPlayer) {
                    world_set_block(hit.placeX, hit.placeY, hit.placeZ, blockToPlace);
                    inventory_remove_current(&inventory);
                    audio_play_sfx(SFX_PLACE);
                    swingTimer = 8;
                }
            }
        }

        if (swingTimer > 0) swingTimer--;

        // =========================================================
        // TOP SCREEN: 3D WORLD RENDER
        // =========================================================
        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        gluPerspective(65.0f, 256.0f / 192.0f, 0.1f, 40.0f);

        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();
        camera_apply_to_gl(&camera);

        // Configure opaque polygons with POLY_ALPHA(31)
        glPolyFmt(POLY_ALPHA(31) | POLY_CULL_NONE | POLY_ID(0));

        // Bind Texture Atlas
        glBindTexture(0, textures_get_id());

        // Render visible voxel terrain with face culling
        world_render(camera.x, camera.y, camera.z);

        // Render First-Person Hand / Tool in bottom right corner
        render_first_person_hand(inventory_get_selected_type(&inventory), swingTimer, walkBob);

        // Draw Top Screen Center Crosshair directly in front of camera
        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();
        glBindTexture(0, 0); // No texture for solid color crosshair
        glColor3b(31, 31, 31);
        glPolyFmt(POLY_ALPHA(31) | POLY_CULL_NONE | POLY_ID(1));
        glBegin(GL_QUADS);
            v16 chW = floattov16(0.003f);
            v16 chH = floattov16(0.025f);
            v16 chZ = floattov16(-0.25f);
            // Vertical bar
            glVertex3v16(-chW,  chH, chZ);
            glVertex3v16( chW,  chH, chZ);
            glVertex3v16( chW, -chH, chZ);
            glVertex3v16(-chW, -chH, chZ);
            // Horizontal bar
            glVertex3v16(-chH,  chW, chZ);
            glVertex3v16( chH,  chW, chZ);
            glVertex3v16( chH, -chW, chZ);
            glVertex3v16(-chH, -chW, chZ);
        glEnd();

        glFlush(0);

        // =========================================================
        // BOTTOM SCREEN: 2D TOUCH HUD & CRAFTING TABLE
        // =========================================================
        ui_render(&ui, &inventory, &hit);
    }

    return 0;
}
