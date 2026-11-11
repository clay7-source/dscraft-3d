#ifndef INVENTORY_H
#define INVENTORY_H

#include <nds.h>
#include "world.h"

typedef enum {
    ITEM_NONE = 0,
    ITEM_DIRT,
    ITEM_WOOD,
    ITEM_STONE,
    ITEM_PLANKS,
    ITEM_STICKS,
    ITEM_WOOD_PICK,
    ITEM_STONE_PICK,
    ITEM_TORCH,
    NUM_ITEM_TYPES
} ItemType;

typedef struct {
    ItemType type;
    int count;
} ItemStack;

#define HOTBAR_SLOTS 5
#define CRAFT_GRID_SLOTS 4 // 2x2

typedef struct {
    ItemStack hotbar[HOTBAR_SLOTS];
    int selectedSlot;

    // 2x2 Crafting Grid
    ItemStack craftGrid[CRAFT_GRID_SLOTS];
    ItemStack craftOutput;

    // Backpack general inventory
    ItemStack backpack[8];
} Inventory;

void inventory_init(Inventory *inv);
bool inventory_add_item(Inventory *inv, ItemType type, int count);
bool inventory_remove_current(Inventory *inv);
ItemType inventory_get_selected_type(const Inventory *inv);
BlockType item_to_block(ItemType item);
ItemType block_to_item(BlockType block);
const char* item_get_name(ItemType type);

void inventory_check_recipes(Inventory *inv);
bool inventory_craft(Inventory *inv);

#endif // INVENTORY_H
