#include "camera.h"
#include "world.h"
#include <math.h>

#define PITCH_LIMIT 1.40f // ~80 degrees

void camera_init(Camera *cam, float startX, float startY, float startZ) {
    cam->x = startX;
    cam->y = startY;
    cam->z = startZ;
    cam->yaw = 0.0f;
    cam->pitch = 0.0f;
    cam->velY = 0.0f;
    cam->onGround = false;
}

void camera_get_direction(const Camera *cam, float *dx, float *dy, float *dz) {
    float cosPitch = cosf(cam->pitch);
    *dx = cosPitch * sinf(cam->yaw);
    *dy = sinf(cam->pitch);
    *dz = cosPitch * -cosf(cam->yaw);
}

void camera_update(Camera *cam, u32 keysHeld, u32 keysDown,
                   int deltaTouchX, int deltaTouchY) {
    // 1. Stylus Camera Rotation (mouse-look)
    if (deltaTouchX != 0 || deltaTouchY != 0) {
        cam->yaw += deltaTouchX * 0.009f;
        cam->pitch -= deltaTouchY * 0.009f;

        if (cam->pitch > PITCH_LIMIT)  cam->pitch = PITCH_LIMIT;
        if (cam->pitch < -PITCH_LIMIT) cam->pitch = -PITCH_LIMIT;
    }

    // 2. D-Pad Movement Vectors
    float moveSpeed = 0.12f;
    float forwardX = sinf(cam->yaw) * moveSpeed;
    float forwardZ = -cosf(cam->yaw) * moveSpeed;
    float strafeX  = cosf(cam->yaw) * moveSpeed;
    float strafeZ  = sinf(cam->yaw) * moveSpeed;

    float newX = cam->x;
    float newZ = cam->z;

    if (keysHeld & KEY_UP) {
        newX += forwardX;
        newZ += forwardZ;
    }
    if (keysHeld & KEY_DOWN) {
        newX -= forwardX;
        newZ -= forwardZ;
    }
    if (keysHeld & KEY_LEFT) {
        newX -= strafeX;
        newZ -= strafeZ;
    }
    if (keysHeld & KEY_RIGHT) {
        newX += strafeX;
        newZ += strafeZ;
    }

    // Horizontal wall collision (check body height at y - 0.5 and y - 1.2)
    int bodyY = (int)floorf(cam->y - 1.0f);
    if (!world_is_solid((int)floorf(newX), bodyY, (int)floorf(cam->z))) {
        cam->x = newX;
    }
    if (!world_is_solid((int)floorf(cam->x), bodyY, (int)floorf(newZ))) {
        cam->z = newZ;
    }

    // 3. Jump (B or X button)
    if ((keysDown & (KEY_B | KEY_X)) && cam->onGround) {
        cam->velY = 0.22f;
        cam->onGround = false;
    }

    // 4. Gravity
    cam->velY -= 0.012f;
    if (cam->velY < -0.40f) cam->velY = -0.40f; // Terminal velocity
    cam->y += cam->velY;

    // 5. Ground Collision Check
    float footY = cam->y - 1.65f;
    int groundBlockY = (int)floorf(footY);
    int groundBlockX = (int)floorf(cam->x);
    int groundBlockZ = (int)floorf(cam->z);

    if (world_is_solid(groundBlockX, groundBlockY, groundBlockZ)) {
        cam->y = (float)(groundBlockY + 1) + 1.65f;
        cam->velY = 0.0f;
        cam->onGround = true;
    } else {
        cam->onGround = false;
    }

    // Keep within world bounds
    if (cam->x < 1.0f) cam->x = 1.0f;
    if (cam->x > WORLD_SIZE_X - 1.5f) cam->x = WORLD_SIZE_X - 1.5f;
    if (cam->z < 1.0f) cam->z = 1.0f;
    if (cam->z > WORLD_SIZE_Z - 1.5f) cam->z = WORLD_SIZE_Z - 1.5f;
    if (cam->y < 2.0f) { cam->y = 2.0f; cam->velY = 0.0f; cam->onGround = true; }
}

void camera_apply_to_gl(const Camera *cam) {
    float dx, dy, dz;
    camera_get_direction(cam, &dx, &dy, &dz);

    gluLookAt(0.0f, 0.0f, 0.0f,
              dx, dy, dz,
              0.0f, 1.0f, 0.0f);
}
