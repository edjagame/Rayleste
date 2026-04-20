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
    std::vector<Screen> screens;
};

struct KeybindSettings {
    KeyboardKey jump = KEY_SPACE;
    KeyboardKey dash = KEY_ENTER;
    KeyboardKey grab = KEY_LEFT_SHIFT;
    KeyboardKey up = KEY_UP;
    KeyboardKey down = KEY_DOWN;
    KeyboardKey left = KEY_LEFT;
    KeyboardKey right = KEY_RIGHT;
};

struct SoundSettings {
    std::string jump = "assets/sounds/jump.wav";
    std::string dash = "assets/sounds/dash.wav";
    std::string death = "assets/sounds/death.wav";
    std::string bgm = "assets/sounds/bgm.wav";
};

struct Settings {
    TilemapSettings tilemap;
    KeybindSettings keybinds;
    SoundSettings sounds;
};

Settings LoadSettings(const std::string& filepath = "settings.json");
