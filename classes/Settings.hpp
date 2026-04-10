#pragma once

#include <raylib.h>
#include <string>
#include <vector>

#include "Grid.hpp"

struct TilemapSettings {
    std::string image_filename;
    Vector2 tile_size_sprite_sheet;
    Vector2 tile_size_grid;
    std::vector<Tile> tiles;
    Vector2 grid_coords;
    std::vector<int> grid_data;
};

struct KeybindSettings {
    KeyboardKey jump = KEY_SPACE;
    KeyboardKey dash = KEY_ENTER;
    KeyboardKey up = KEY_UP;
    KeyboardKey down = KEY_DOWN;
    KeyboardKey left = KEY_LEFT;
    KeyboardKey right = KEY_RIGHT;
};

struct Settings {
    TilemapSettings tilemap;
    KeybindSettings keybinds;
};

Settings LoadSettings(const std::string& filepath = "settings.json");
