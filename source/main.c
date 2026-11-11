#include <nds.h>
#include <stdio.h>
#include <math.h>

#include "world.h"
#include "camera.h"
#include "textures.h"
#include "inventory.h"
#include "ui.h"

int main(void) {
    // 1. Power on 2D and 3D graphics hardware
    powerOn(POWER_ALL_2D | POWER_3D_CORE | POWER_MATRIX);

    // 2. Setup Top Screen for 3D OpenGL Rendering
    videoSetMode(MODE_0_3D);
    vramSetBankA(VRAM_A_TEXTURE); // VRAM Bank A for 3D Textures

    glInit();
    glEnable(GL_TEXTURE_2D | GL_ANTIALIAS);

    // Daylight sky background color (Light Blue)
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

    Camera camera;
    camera_init(&camera, 16.0f, 15.0f, 16.0f);

    Inventory inventory;
    inventory_init(&inventory);

    UIState ui;
    ui.mode = UI_MODE_GAME;
    ui.requestMine = false;
    ui.requestPlace = false;
    ui.requestJump = false;

    touchPosition lastTouch = {0, 0};
    bool wasTouching = false;

    // Main Game Loop (Locked at 60 FPS via VBlank)
    while (pmMainLoop()) {
        swiWaitForVBlank();

        scanKeys();
        u32 kHeld = keysHeld();
        u32 kDown = keysDown();

        touchPosition touch;
        touchRead(&touch);

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
                }
            }
        }

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

        // Bind 16x16 Texture Atlas
        glBindTexture(0, textures_get_id());

        // Render visible voxel terrain with face culling
        world_render(camera.x, camera.y, camera.z);

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
