#ifndef CAMERA_H
#define CAMERA_H

#include <nds.h>

typedef struct {
    float x, y, z;      // Player eye position
    float yaw;          // Horizontal rotation (radians)
    float pitch;        // Vertical rotation (radians)
    float velY;         // Vertical velocity (gravity/jump)
    bool onGround;      // Grounded state
} Camera;

void camera_init(Camera *cam, float startX, float startY, float startZ);
void camera_update(Camera *cam, u32 keysHeld, u32 keysDown,
                   int deltaTouchX, int deltaTouchY);
void camera_get_direction(const Camera *cam, float *dx, float *dy, float *dz);
void camera_apply_to_gl(const Camera *cam);

#endif // CAMERA_H
