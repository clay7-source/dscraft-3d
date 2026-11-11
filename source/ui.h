#ifndef UI_H
#define UI_H

#include <nds.h>
#include "inventory.h"
#include "world.h"

typedef enum {
    UI_MODE_GAME = 0,
    UI_MODE_CRAFT
} UIMode;

typedef struct {
    UIMode mode;
    bool requestMine;
    bool requestPlace;
    bool requestJump;
} UIState;

void ui_init(void);
void ui_update(UIState *ui, Inventory *inv, touchPosition *touch,
               u32 keysDown, u32 keysHeld);
void ui_render(const UIState *ui, const Inventory *inv, const RaycastHit *hit);

#endif // UI_H
