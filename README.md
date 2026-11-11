# DS-Craft 3D (Nintendo DS Homebrew)

A custom 3D voxel sandbox game built in C for the **Nintendo DS / DS Lite**, featuring 3D first-person terrain rendering on the top screen and a touchscreen HUD + 2x2 Crafting Table on the bottom screen.

---

## Features

- **Hardware 3D Engine (Top Screen)**:
  - Textured 3D voxel cubes (Grass, Dirt, Stone, Wood, Leaves, Planks, Bedrock).
  - Procedural rolling hills terrain generation with trees and ores.
  - Directional lighting and face culling (optimized for 60 FPS on DS Lite ARM9 hardware).
  - First-person 3D camera with realistic gravity, jumping, and collision detection.
  - In-engine raycasting for block targeting, mining, and placement.

- **Touchscreen HUD & Crafting (Bottom Screen)**:
  - **Stylus Mouse-Look**: Drag your stylus in the touchpad area to aim smoothly in 360°.
  - **5-Slot Hotbar**: Tap any slot to select your active tool or block.
  - **On-Screen Touch Buttons**: Quick `[MINE]` and `[JUMP]` buttons for one-handed stylus play.
  - **Interactive 2×2 Crafting Table**:
    - Tap `[CRAFT]` to open the crafting menu.
    - Tap items to move them into the 2×2 grid.
    - Tap the output slot to craft!
    - Tap `[CLOSE]` to return to the 3D world.

---

## Controls (DS Lite)

| Control | Action |
|---|---|
| **D-Pad Up / Down** | Walk Forward / Backward |
| **D-Pad Left / Right** | Strafe Left / Right |
| **Stylus Drag (Bottom Screen)** | 3D Camera Look (Mouse-look) |
| **L Button** (or Touch `[MINE]`) | Mine targeted block |
| **R Button** | Place selected block |
| **B Button / X Button** (or Touch `[JUMP]`) | Jump |
| **Touch Hotbar Slots (1 - 5)** | Switch equipped item |
| **Touch `[CRAFT]` Button** | Open / Close Crafting Table |

---

## Crafting Recipes

| Ingredients (2×2 Grid) | Result |
|---|---|
| 1 Wood Trunk | 4 Wood Planks |
| 2 Wood Planks (vertical) | 4 Sticks |
| 2 Wood Planks + 1 Stick | 1 Wooden Pickaxe |
| 2 Cobblestone + 1 Stick | 1 Stone Pickaxe |
| 1 Wood + 1 Stick | 4 Torches |

---

## Running on your DSPico Flashcart

1. Build the ROM or download **`dscraft3d.nds`**.
2. Insert your DSPico's microSD card into your computer.
3. Copy **`dscraft3d.nds`** onto the root or `games/` folder of the microSD card.
4. Insert the microSD card back into your DSPico, insert the DSPico into Slot-1 of your Nintendo DS Lite, turn on the console, and launch **DS-Craft 3D**!

---

## Building the .NDS ROM

### Option A: Via GitHub Actions (Zero Local Setup)
Push this repository to GitHub. The included workflow (`.github/workflows/build-nds.yml`) will automatically compile the code using `devkitarm` and provide the downloadable `.nds` file under the **Actions** tab.

### Option B: Local Build on macOS (devkitPro)
1. Install [devkitPro Pacman for macOS](https://github.com/devkitPro/pacman/releases/latest).
2. Install the NDS development packages:
   ```bash
   sudo dkp-pacman -S nds-dev
   ```
3. Set your environment variables in `~/.zshrc`:
   ```bash
   export DEVKITPRO=/opt/devkitpro
   export DEVKITARM=${DEVKITPRO}/devkitARM
   export PATH=${DEVKITPRO}/tools/bin:$PATH
   ```
4. In this directory, run:
   ```bash
   make
   ```
   This generates **`dscraft3d.nds`**.
