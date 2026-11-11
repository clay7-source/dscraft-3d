#include "inventory.h"
#include <string.h>

static const char* ITEM_NAMES[NUM_ITEM_TYPES] = {
    "Empty",
    "Dirt",
    "Wood Trunk",
    "Cobblestone",
    "Wood Planks",
    "Sticks",
    "Wood Pickaxe",
    "Stone Pickaxe",
    "Torch",
    "Sand",
    "Coal",
    "Iron Ingot",
    "Glass",
    "Craft Table"
};

const char* item_get_name(ItemType type) {
    if (type >= 0 && type < NUM_ITEM_TYPES) {
        return ITEM_NAMES[type];
    }
    return "Unknown";
}

BlockType item_to_block(ItemType item) {
    switch (item) {
        case ITEM_DIRT:           return BLOCK_DIRT;
        case ITEM_WOOD:           return BLOCK_WOOD;
        case ITEM_STONE:          return BLOCK_STONE;
        case ITEM_PLANKS:         return BLOCK_PLANKS;
        case ITEM_SAND:           return BLOCK_SAND;
        case ITEM_GLASS:          return BLOCK_GLASS;
        case ITEM_CRAFTING_TABLE: return BLOCK_CRAFTING_TABLE;
        default:                  return BLOCK_AIR;
    }
}

ItemType block_to_item(BlockType block) {
    switch (block) {
        case BLOCK_GRASS:
        case BLOCK_DIRT:           return ITEM_DIRT;
        case BLOCK_WOOD:           return ITEM_WOOD;
        case BLOCK_STONE:
        case BLOCK_COBBLE:         return ITEM_STONE;
        case BLOCK_PLANKS:         return ITEM_PLANKS;
        case BLOCK_LEAVES:         return ITEM_STICKS;
        case BLOCK_SAND:           return ITEM_SAND;
        case BLOCK_COAL_ORE:       return ITEM_COAL;
        case BLOCK_IRON_ORE:       return ITEM_IRON;
        case BLOCK_GLASS:          return ITEM_GLASS;
        case BLOCK_CRAFTING_TABLE: return ITEM_CRAFTING_TABLE;
        default:                   return ITEM_NONE;
    }
}

void inventory_init(Inventory *inv) {
    memset(inv, 0, sizeof(Inventory));
    inv->selectedSlot = 0;

    // Starter inventory
    inv->hotbar[0].type = ITEM_WOOD_PICK;
    inv->hotbar[0].count = 1;

    inv->hotbar[1].type = ITEM_DIRT;
    inv->hotbar[1].count = 16;

    inv->hotbar[2].type = ITEM_WOOD;
    inv->hotbar[2].count = 8;
}

bool inventory_add_item(Inventory *inv, ItemType type, int count) {
    if (type == ITEM_NONE || count <= 0) return false;

    // 1. Try to stack into existing hotbar slot
    for (int i = 0; i < HOTBAR_SLOTS; i++) {
        if (inv->hotbar[i].type == type && inv->hotbar[i].count < 64) {
            inv->hotbar[i].count += count;
            return true;
        }
    }

    // 2. Try empty hotbar slot
    for (int i = 0; i < HOTBAR_SLOTS; i++) {
        if (inv->hotbar[i].type == ITEM_NONE) {
            inv->hotbar[i].type = type;
            inv->hotbar[i].count = count;
            return true;
        }
    }

    // 3. Try to stack into backpack
    for (int i = 0; i < 8; i++) {
        if (inv->backpack[i].type == type && inv->backpack[i].count < 64) {
            inv->backpack[i].count += count;
            return true;
        }
    }

    // 4. Try empty backpack slot
    for (int i = 0; i < 8; i++) {
        if (inv->backpack[i].type == ITEM_NONE) {
            inv->backpack[i].type = type;
            inv->backpack[i].count = count;
            return true;
        }
    }

    return false; // Inventory full
}

bool inventory_remove_current(Inventory *inv) {
    int s = inv->selectedSlot;
    if (inv->hotbar[s].count > 0) {
        inv->hotbar[s].count--;
        if (inv->hotbar[s].count == 0) {
            inv->hotbar[s].type = ITEM_NONE;
        }
        return true;
    }
    return false;
}

ItemType inventory_get_selected_type(const Inventory *inv) {
    return inv->hotbar[inv->selectedSlot].type;
}

void inventory_check_recipes(Inventory *inv) {
    // Reset craft output
    inv->craftOutput.type = ITEM_NONE;
    inv->craftOutput.count = 0;

    int totalWood = 0;
    int totalPlanks = 0;
    int totalSticks = 0;
    int totalStone = 0;
    int totalCoal = 0;
    int totalItems = 0;

    for (int i = 0; i < CRAFT_GRID_SLOTS; i++) {
        if (inv->craftGrid[i].count > 0) {
            totalItems++;
            if (inv->craftGrid[i].type == ITEM_WOOD) totalWood++;
            if (inv->craftGrid[i].type == ITEM_PLANKS) totalPlanks++;
            if (inv->craftGrid[i].type == ITEM_STICKS) totalSticks++;
            if (inv->craftGrid[i].type == ITEM_STONE) totalStone++;
            if (inv->craftGrid[i].type == ITEM_COAL) totalCoal++;
        }
    }

    // Recipe 1: 1 Wood -> 4 Planks
    if (totalWood == 1 && totalItems == 1) {
        inv->craftOutput.type = ITEM_PLANKS;
        inv->craftOutput.count = 4;
        return;
    }

    // Recipe 2: 4 Planks in 2x2 grid -> 1 Crafting Table
    if (totalPlanks == 4 && totalItems == 4) {
        inv->craftOutput.type = ITEM_CRAFTING_TABLE;
        inv->craftOutput.count = 1;
        return;
    }

    // Recipe 3: 2 Planks -> 4 Sticks
    if (totalPlanks == 2 && totalItems == 2) {
        inv->craftOutput.type = ITEM_STICKS;
        inv->craftOutput.count = 4;
        return;
    }

    // Recipe 4: 2 Planks + 1 Stick -> Wooden Pickaxe
    if (totalPlanks >= 2 && totalSticks >= 1 && totalItems == 3) {
        inv->craftOutput.type = ITEM_WOOD_PICK;
        inv->craftOutput.count = 1;
        return;
    }

    // Recipe 5: 2 Stone + 1 Stick -> Stone Pickaxe
    if (totalStone >= 2 && totalSticks >= 1 && totalItems == 3) {
        inv->craftOutput.type = ITEM_STONE_PICK;
        inv->craftOutput.count = 1;
        return;
    }

    // Recipe 6: 1 Coal/Wood + 1 Stick -> 4 Torches
    if ((totalCoal == 1 || totalWood == 1) && totalSticks == 1 && totalItems == 2) {
        inv->craftOutput.type = ITEM_TORCH;
        inv->craftOutput.count = 4;
        return;
    }
}

bool inventory_craft(Inventory *inv) {
    if (inv->craftOutput.type == ITEM_NONE || inv->craftOutput.count <= 0) {
        return false;
    }

    // Give crafted item to inventory
    if (!inventory_add_item(inv, inv->craftOutput.type, inv->craftOutput.count)) {
        return false; // No room
    }

    // Consume 1 ingredient from each occupied craft slot
    for (int i = 0; i < CRAFT_GRID_SLOTS; i++) {
        if (inv->craftGrid[i].count > 0) {
            inv->craftGrid[i].count--;
            if (inv->craftGrid[i].count == 0) {
                inv->craftGrid[i].type = ITEM_NONE;
            }
        }
    }

    inventory_check_recipes(inv);
    return true;
}
