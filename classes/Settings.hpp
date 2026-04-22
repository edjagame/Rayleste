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
    std::string bgm = "assets/sounds/bgm.mp3";
    std::string win_music = "assets/sounds/win_music.mp3";
};

struct PlayerAnimationSettings {
    std::string idle = "assets/player/PlayerAnimations/idle.png";
    std::string run = "assets/player/PlayerAnimations/run.png";
    std::string jump = "assets/player/PlayerAnimations/jump.png";
    std::string fall = "assets/player/PlayerAnimations/fall.png";
    std::string fall_pose = "assets/player/PlayerAnimations/fall_pose.png";
    std::string dash = "assets/player/PlayerAnimations/dash.png";
    std::string death = "assets/player/PlayerAnimations/death.png";
};

struct TextureSettings {
    std::string background = "assets/bg.png";
};

struct Settings {
    TilemapSettings tilemap;
    KeybindSettings keybinds;
    SoundSettings sounds;
    PlayerAnimationSettings player_animations;
    TextureSettings textures;
    Vector2 checkpoint_save_data;
};

Settings LoadSettings(const std::string& filepath = "settings.json");
void LoadSave(const std::string& filepath, Settings& settings);
void SaveSettings(const Settings& settings, const std::string& filepath = "settings.json");