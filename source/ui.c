#include "ui.h"
#include <string.h>

static u16 *sub_framebuffer = NULL;

// Palette definitions (RGB15: 0..31 per channel, bit 15 set)
#define C_BLACK       (RGB15(0, 0, 0) | (1 << 15))
#define C_WHITE       (RGB15(31, 31, 31) | (1 << 15))
#define C_DARK_GRAY   (RGB15(6, 6, 7) | (1 << 15))
#define C_MID_GRAY    (RGB15(14, 14, 15) | (1 << 15))
#define C_LIGHT_GRAY  (RGB15(22, 22, 23) | (1 << 15))
#define C_SLATE       (RGB15(9, 10, 12) | (1 << 15))
#define C_RED         (RGB15(31, 2, 2) | (1 << 15))
#define C_DARK_RED    (RGB15(16, 2, 2) | (1 << 15))
#define C_GOLD        (RGB15(31, 26, 4) | (1 << 15))
#define C_BROWN       (RGB15(16, 10, 5) | (1 << 15))
#define C_WOOD        (RGB15(22, 16, 8) | (1 << 15))
#define C_GREEN       (RGB15(6, 24, 6) | (1 << 15))
#define C_CYAN        (RGB15(8, 26, 28) | (1 << 15))
#define C_BLUE        (RGB15(6, 12, 26) | (1 << 15))

static inline void set_sub_pixel(int x, int y, u16 color) {
    if (x >= 0 && x < 256 && y >= 0 && y < 192) {
        sub_framebuffer[y * 256 + x] = color;
    }
}

static void draw_fill_rect(int x, int y, int w, int h, u16 color) {
    for (int j = 0; j < h; j++) {
        int py = y + j;
        if (py < 0 || py >= 192) continue;
        for (int i = 0; i < w; i++) {
            int px = x + i;
            if (px < 0 || px >= 256) continue;
            sub_framebuffer[py * 256 + px] = color;
        }
    }
}

static void draw_rect_border(int x, int y, int w, int h, u16 color) {
    for (int i = 0; i < w; i++) {
        set_sub_pixel(x + i, y, color);
        set_sub_pixel(x + i, y + h - 1, color);
    }
    for (int j = 0; j < h; j++) {
        set_sub_pixel(x, y + j, color);
        set_sub_pixel(x + w - 1, y + j, color);
    }
}

// 3x5 Minimal Bitmap Font
static const u8 FONT_3X5[][3] = {
    [' '] = {0x00, 0x00, 0x00},
    ['0'] = {0x1F, 0x11, 0x1F},
    ['1'] = {0x00, 0x1F, 0x00},
    ['2'] = {0x17, 0x15, 0x1D},
    ['3'] = {0x15, 0x15, 0x1F},
    ['4'] = {0x1C, 0x04, 0x1F},
    ['5'] = {0x1D, 0x15, 0x17},
    ['6'] = {0x1F, 0x15, 0x17},
    ['7'] = {0x01, 0x01, 0x1F},
    ['8'] = {0x1F, 0x15, 0x1F},
    ['9'] = {0x1D, 0x15, 0x1F},
    ['A'] = {0x1E, 0x05, 0x1E},
    ['B'] = {0x1F, 0x15, 0x0A},
    ['C'] = {0x0E, 0x11, 0x11},
    ['D'] = {0x1F, 0x11, 0x0E},
    ['E'] = {0x1F, 0x15, 0x15},
    ['F'] = {0x1F, 0x05, 0x05},
    ['G'] = {0x0E, 0x11, 0x1D},
    ['H'] = {0x1F, 0x04, 0x1F},
    ['I'] = {0x11, 0x1F, 0x11},
    ['J'] = {0x08, 0x10, 0x0F},
    ['K'] = {0x1F, 0x04, 0x1B},
    ['L'] = {0x1F, 0x10, 0x10},
    ['M'] = {0x1F, 0x02, 0x1F},
    ['N'] = {0x1F, 0x06, 0x1F},
    ['O'] = {0x0E, 0x11, 0x0E},
    ['P'] = {0x1F, 0x05, 0x02},
    ['R'] = {0x1F, 0x05, 0x1A},
    ['S'] = {0x12, 0x15, 0x09},
    ['T'] = {0x01, 0x1F, 0x01},
    ['U'] = {0x0F, 0x10, 0x0F},
    ['V'] = {0x07, 0x18, 0x07},
    ['W'] = {0x1F, 0x08, 0x1F},
    ['X'] = {0x1B, 0x04, 0x1B},
    ['Y'] = {0x03, 0x1C, 0x03},
    ['Z'] = {0x19, 0x15, 0x13},
    [':'] = {0x00, 0x0A, 0x00},
    ['-'] = {0x04, 0x04, 0x04},
    ['+'] = {0x04, 0x0E, 0x04},
    ['x'] = {0x0A, 0x04, 0x0A},
};

static void draw_char_3x5(int x, int y, char c, u16 color) {
    if ((u8)c >= sizeof(FONT_3X5) / sizeof(FONT_3X5[0])) return;
    const u8 *glyph = FONT_3X5[(u8)c];
    for (int col = 0; col < 3; col++) {
        u8 bits = glyph[col];
        for (int row = 0; row < 5; row++) {
            if (bits & (1 << row)) {
                set_sub_pixel(x + col, y + row, color);
            }
        }
    }
}

static void draw_text(int x, int y, const char *str, u16 color) {
    while (*str) {
        char c = *str;
        if (c >= 'a' && c <= 'z') c = c - 'a' + 'A';
        draw_char_3x5(x, y, c, color);
        x += 4;
        str++;
    }
}

static void draw_number(int x, int y, int num, u16 color) {
    char buf[12];
    int idx = 0;
    if (num == 0) {
        buf[idx++] = '0';
    } else {
        char temp[12];
        int t = 0;
        while (num > 0 && t < 10) {
            temp[t++] = '0' + (num % 10);
            num /= 10;
        }
        for (int i = t - 1; i >= 0; i--) {
            buf[idx++] = temp[i];
        }
    }
    buf[idx] = '\0';
    draw_text(x, y, buf, color);
}

// Draw Heart icon (7x6 pixels)
static void draw_heart(int x, int y, bool full) {
    static const u8 HEART_BM[6] = {
        0b01101100,
        0b11111110,
        0b11111110,
        0b01111100,
        0b00111000,
        0b00010000
    };
    for (int row = 0; row < 6; row++) {
        u8 line = HEART_BM[row];
        for (int col = 0; col < 8; col++) {
            if (line & (1 << (7 - col))) {
                set_sub_pixel(x + col, y + row, full ? C_RED : C_DARK_RED);
            }
        }
    }
}

// Draw 14x14 Item Icon inside UI slots
static void draw_item_icon(int x, int y, ItemType type) {
    switch (type) {
        case ITEM_DIRT:
            draw_fill_rect(x + 2, y + 2, 10, 10, C_BROWN);
            set_sub_pixel(x + 4, y + 4, C_DARK_RED);
            set_sub_pixel(x + 8, y + 7, C_DARK_RED);
            break;
        case ITEM_WOOD:
            draw_fill_rect(x + 3, y + 1, 8, 12, C_BROWN);
            draw_fill_rect(x + 4, y + 2, 6, 2, C_WOOD);
            draw_rect_border(x + 3, y + 1, 8, 12, C_DARK_GRAY);
            break;
        case ITEM_STONE:
            draw_fill_rect(x + 2, y + 2, 10, 10, C_MID_GRAY);
            draw_rect_border(x + 2, y + 2, 10, 10, C_LIGHT_GRAY);
            set_sub_pixel(x + 5, y + 5, C_DARK_GRAY);
            set_sub_pixel(x + 8, y + 8, C_DARK_GRAY);
            break;
        case ITEM_PLANKS:
            draw_fill_rect(x + 2, y + 2, 10, 10, C_WOOD);
            for (int i = 0; i < 10; i++) {
                set_sub_pixel(x + 2 + i, y + 5, C_BROWN);
                set_sub_pixel(x + 2 + i, y + 8, C_BROWN);
            }
            break;
        case ITEM_STICKS:
            for (int i = 0; i < 8; i++) {
                set_sub_pixel(x + 3 + i, y + 11 - i, C_BROWN);
                set_sub_pixel(x + 4 + i, y + 11 - i, C_WOOD);
            }
            break;
        case ITEM_WOOD_PICK:
            // Handle
            for (int i = 0; i < 7; i++) {
                set_sub_pixel(x + 3 + i, y + 11 - i, C_BROWN);
            }
            // Pick head (Wood)
            draw_fill_rect(x + 6, y + 2, 6, 3, C_WOOD);
            set_sub_pixel(x + 5, y + 4, C_WOOD);
            set_sub_pixel(x + 11, y + 4, C_WOOD);
            break;
        case ITEM_STONE_PICK:
            // Handle
            for (int i = 0; i < 7; i++) {
                set_sub_pixel(x + 3 + i, y + 11 - i, C_BROWN);
            }
            // Pick head (Stone)
            draw_fill_rect(x + 6, y + 2, 6, 3, C_LIGHT_GRAY);
            set_sub_pixel(x + 5, y + 4, C_LIGHT_GRAY);
            set_sub_pixel(x + 11, y + 4, C_LIGHT_GRAY);
            break;
        case ITEM_TORCH:
            // Stick
            for (int i = 0; i < 6; i++) {
                set_sub_pixel(x + 6, y + 6 + i, C_BROWN);
            }
            // Flame
            draw_fill_rect(x + 5, y + 3, 3, 3, C_GOLD);
            set_sub_pixel(x + 6, y + 2, C_RED);
            break;
        default:
            break;
    }
}

void ui_init(void) {
    videoSetModeSub(MODE_5_2D);
    vramSetBankC(VRAM_C_SUB_BG);
    int bgSub = bgInitSub(3, BgType_Bmp16, BgSize_B16_256x256, 0, 0);
    sub_framebuffer = (u16*)bgGetGfxPtr(bgSub);
    memset(sub_framebuffer, 0, 256 * 192 * sizeof(u16));
}

void ui_update(UIState *ui, Inventory *inv, touchPosition *touch,
               u32 keysDown, u32 keysHeld) {
    ui->requestMine = false;
    ui->requestPlace = false;
    ui->requestJump = false;

    // Physical Buttons
    if (keysDown & KEY_L) ui->requestMine = true;
    if (keysDown & KEY_R) ui->requestPlace = true;
    if (keysDown & (KEY_B | KEY_X)) ui->requestJump = true;

    // Touch Screen Interactions
    if (keysDown & KEY_TOUCH) {
        int tx = touch->px;
        int ty = touch->py;

        if (ui->mode == UI_MODE_GAME) {
            // 1. [CRAFT] Button (Top-Right)
            if (tx >= 188 && tx <= 250 && ty >= 2 && ty <= 18) {
                ui->mode = UI_MODE_CRAFT;
                return;
            }

            // 2. [MINE] Touch Button (Bottom-Left)
            if (tx >= 6 && tx <= 48 && ty >= 105 && ty <= 140) {
                ui->requestMine = true;
                return;
            }

            // 3. [JUMP] Touch Button (Bottom-Right)
            if (tx >= 208 && tx <= 250 && ty >= 105 && ty <= 140) {
                ui->requestJump = true;
                return;
            }

            // 4. Hotbar Slot selection (Bottom bar: Y: 152..188)
            if (ty >= 150 && ty <= 190) {
                for (int i = 0; i < HOTBAR_SLOTS; i++) {
                    int slotX = 24 + i * 44;
                    if (tx >= slotX && tx <= slotX + 34) {
                        inv->selectedSlot = i;
                        return;
                    }
                }
            }
        } else if (ui->mode == UI_MODE_CRAFT) {
            // [CLOSE] Button
            if (tx >= 196 && tx <= 250 && ty >= 6 && ty <= 22) {
                ui->mode = UI_MODE_GAME;
                return;
            }

            // Tap Output Slot to Craft (X: 130..164, Y: 56..90)
            if (tx >= 130 && tx <= 164 && ty >= 56 && ty <= 90) {
                inventory_craft(inv);
                return;
            }

            // Tap 2x2 Crafting Grid slots to return item to inventory
            int craftStartX = 34, craftStartY = 42;
            for (int r = 0; r < 2; r++) {
                for (int c = 0; c < 2; c++) {
                    int slotIdx = r * 2 + c;
                    int sx = craftStartX + c * 38;
                    int sy = craftStartY + r * 38;
                    if (tx >= sx && tx <= sx + 32 && ty >= sy && ty <= sy + 32) {
                        if (inv->craftGrid[slotIdx].count > 0) {
                            inventory_add_item(inv, inv->craftGrid[slotIdx].type, 1);
                            inv->craftGrid[slotIdx].count--;
                            if (inv->craftGrid[slotIdx].count == 0) {
                                inv->craftGrid[slotIdx].type = ITEM_NONE;
                            }
                            inventory_check_recipes(inv);
                        }
                        return;
                    }
                }
            }

            // Tap Inventory / Hotbar item to place 1 into crafting grid
            // Hotbar row (Y: 150..182)
            if (ty >= 150 && ty <= 182) {
                for (int i = 0; i < HOTBAR_SLOTS; i++) {
                    int sx = 24 + i * 44;
                    if (tx >= sx && tx <= sx + 34) {
                        if (inv->hotbar[i].count > 0) {
                            // Find first slot in 2x2 grid that is empty or matches
                            ItemType t = inv->hotbar[i].type;
                            for (int g = 0; g < CRAFT_GRID_SLOTS; g++) {
                                if (inv->craftGrid[g].type == ITEM_NONE ||
                                    (inv->craftGrid[g].type == t && inv->craftGrid[g].count < 64)) {
                                    inv->craftGrid[g].type = t;
                                    inv->craftGrid[g].count++;
                                    inv->hotbar[i].count--;
                                    if (inv->hotbar[i].count == 0) inv->hotbar[i].type = ITEM_NONE;
                                    inventory_check_recipes(inv);
                                    break;
                                }
                            }
                        }
                        return;
                    }
                }
            }
        }
    }
}

void ui_render(const UIState *ui, const Inventory *inv, const RaycastHit *hit) {
    if (!sub_framebuffer) return;

    if (ui->mode == UI_MODE_GAME) {
        // Clear background
        draw_fill_rect(0, 0, 256, 192, C_DARK_GRAY);

        // --- Top Bar (Hearts + Info) ---
        draw_fill_rect(0, 0, 256, 22, C_SLATE);
        draw_rect_border(0, 0, 256, 22, C_MID_GRAY);

        // 5 Health Hearts
        for (int i = 0; i < 5; i++) {
            draw_heart(8 + i * 11, 8, true);
        }

        // Selected Item Name
        ItemType selType = inventory_get_selected_type(inv);
        draw_text(74, 9, item_get_name(selType), C_WHITE);

        // [CRAFT] Button
        draw_fill_rect(190, 4, 60, 14, C_WOOD);
        draw_rect_border(190, 4, 60, 14, C_GOLD);
        draw_text(202, 8, "CRAFT", C_WHITE);

        // --- Touchpad Look Area (Middle) ---
        draw_rect_border(54, 30, 148, 110, C_MID_GRAY);
        draw_text(86, 76, "TOUCHPAD LOOK", C_LIGHT_GRAY);
        draw_text(72, 88, "(DRAG STYLUS TO AIM)", C_MID_GRAY);

        // Raycast Aim Info
        if (hit && hit->hit) {
            draw_text(92, 126, "AIM: ", C_GOLD);
            draw_text(112, 126, item_get_name(block_to_item(hit->blockType)), C_WHITE);
        }

        // On-screen [MINE] and [JUMP] Buttons
        draw_fill_rect(6, 105, 42, 32, C_SLATE);
        draw_rect_border(6, 105, 42, 32, C_RED);
        draw_text(13, 118, "MINE", C_WHITE);

        draw_fill_rect(208, 105, 42, 32, C_SLATE);
        draw_rect_border(208, 105, 42, 32, C_CYAN);
        draw_text(215, 118, "JUMP", C_WHITE);

        // --- Bottom Hotbar ---
        draw_fill_rect(0, 148, 256, 44, C_SLATE);
        draw_rect_border(0, 148, 256, 44, C_MID_GRAY);

        for (int i = 0; i < HOTBAR_SLOTS; i++) {
            int sx = 24 + i * 44;
            int sy = 153;
            bool isSel = (i == inv->selectedSlot);

            draw_fill_rect(sx, sy, 34, 34, isSel ? C_MID_GRAY : C_DARK_GRAY);
            draw_rect_border(sx, sy, 34, 34, isSel ? C_GOLD : C_LIGHT_GRAY);
            if (isSel) {
                // Double border for selected slot
                draw_rect_border(sx + 1, sy + 1, 32, 32, C_GOLD);
            }

            const ItemStack *stack = &inv->hotbar[i];
            if (stack->type != ITEM_NONE && stack->count > 0) {
                draw_item_icon(sx + 10, sy + 7, stack->type);
                draw_number(sx + 18, sy + 23, stack->count, C_WHITE);
            }
        }
    } else {
        // --- CRAFTING MENU MODE ---
        draw_fill_rect(0, 0, 256, 192, C_DARK_GRAY);

        // Header
        draw_fill_rect(0, 0, 256, 26, C_SLATE);
        draw_rect_border(0, 0, 256, 26, C_MID_GRAY);
        draw_text(16, 10, "CRAFTING TABLE (2X2)", C_GOLD);

        // [CLOSE] Button
        draw_fill_rect(194, 5, 56, 16, C_DARK_RED);
        draw_rect_border(194, 5, 56, 16, C_WHITE);
        draw_text(205, 10, "CLOSE", C_WHITE);

        // 2x2 Crafting Grid
        int craftStartX = 34, craftStartY = 38;
        for (int r = 0; r < 2; r++) {
            for (int c = 0; c < 2; c++) {
                int slotIdx = r * 2 + c;
                int sx = craftStartX + c * 38;
                int sy = craftStartY + r * 38;

                draw_fill_rect(sx, sy, 32, 32, C_SLATE);
                draw_rect_border(sx, sy, 32, 32, C_MID_GRAY);

                const ItemStack *stack = &inv->craftGrid[slotIdx];
                if (stack->type != ITEM_NONE && stack->count > 0) {
                    draw_item_icon(sx + 9, sy + 6, stack->type);
                    draw_number(sx + 16, sy + 22, stack->count, C_WHITE);
                }
            }
        }

        // Craft Arrow ->
        draw_text(116, 68, "->", C_GOLD);

        // Output Slot
        draw_fill_rect(132, 54, 38, 38, C_SLATE);
        draw_rect_border(132, 54, 38, 38, C_GOLD);
        draw_rect_border(133, 55, 36, 36, C_GOLD);

        if (inv->craftOutput.type != ITEM_NONE && inv->craftOutput.count > 0) {
            draw_item_icon(132 + 12, 54 + 8, inv->craftOutput.type);
            draw_number(132 + 20, 54 + 26, inv->craftOutput.count, C_GOLD);
            draw_text(178, 68, item_get_name(inv->craftOutput.type), C_WHITE);
        } else {
            draw_text(178, 68, "(NO RECIPE)", C_MID_GRAY);
        }

        // Inventory Row Header
        draw_text(24, 136, "TAP ITEM TO ADD TO CRAFTING:", C_LIGHT_GRAY);

        // Bottom Hotbar (Source items)
        for (int i = 0; i < HOTBAR_SLOTS; i++) {
            int sx = 24 + i * 44;
            int sy = 148;

            draw_fill_rect(sx, sy, 34, 34, C_SLATE);
            draw_rect_border(sx, sy, 34, 34, C_LIGHT_GRAY);

            const ItemStack *stack = &inv->hotbar[i];
            if (stack->type != ITEM_NONE && stack->count > 0) {
                draw_item_icon(sx + 10, sy + 7, stack->type);
                draw_number(sx + 18, sy + 23, stack->count, C_WHITE);
            }
        }
    }
}
