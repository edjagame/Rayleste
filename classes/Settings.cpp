#include "Settings.hpp"
#include "json.hpp"

#include <iostream>
#include <fstream>
#include <string>
#include <map>

using json = nlohmann::json;

Vector2 LoadVector2(const json& j) {
    Vector2 vec;
    vec.x = j["x"].get<float>();
    vec.y = j["y"].get<float>();
    return vec;
}

Screen LoadScreen(const json& j) {
    Screen screen;

    screen.world_position = LoadVector2(j["world_position"]);
    screen.dimensions_tiles = LoadVector2(j["dimensions_tiles"]);
    if (j.contains("checkpoint_position")) {
        screen.checkpoint_position = LoadVector2(j["checkpoint_position"]);
    }
    else {
        screen.checkpoint_position = screen.world_position;
    }
    screen.tile_data = j["tile_data"].get<std::vector<int>>();

    return screen;
}

bool LoadTilemapSettings(const json& json_tilemap, TilemapSettings& tilemap_settings) {
    // Load tilemap metadata
    tilemap_settings.image_filename = json_tilemap["image_filename"].get<std::string>();
    tilemap_settings.tile_size_sprite_sheet = LoadVector2(json_tilemap["tile_size_sprite_sheet"]);
    tilemap_settings.tile_size_grid = LoadVector2(json_tilemap["tile_size_grid"]);

    // Load tile definitions
    const json& tiles = json_tilemap["tiles"];
    for (json::const_iterator it = tiles.begin(); it != tiles.end(); ++it) {
        const json& tile = *it;
        const int id = tile["id"].get<int>();
        const Vector2 pos = LoadVector2(tile["pos"]);
        const TileType type = static_cast<TileType>(tile["type"].get<int>());
        tilemap_settings.tiles.push_back({id, pos, type});
    }

    if (!json_tilemap.contains("screens")) {
        std::cerr << "Missing screens in settings.ini" << std::endl;
        return false;
    }

    for (const json& json_screen : json_tilemap["screens"]) {
        tilemap_settings.screens.push_back(LoadScreen(json_screen));
    }

    return true;
}

Color LoadColor(const json& j) {
    Color color;
    color.r = j["r"].get<unsigned char>();
    color.g = j["g"].get<unsigned char>();
    color.b = j["b"].get<unsigned char>();
    color.a = 255;
    return color;
}

// kinda brute force helper function
KeyboardKey StringToKeyboardKey(const std::string& keyName) {
    static const std::map<std::string, KeyboardKey> keyMap = {
        {"KEY_SPACE", KEY_SPACE},
        {"KEY_ENTER", KEY_ENTER},
        {"KEY_UP", KEY_UP},
        {"KEY_DOWN", KEY_DOWN},
        {"KEY_LEFT", KEY_LEFT},
        {"KEY_RIGHT", KEY_RIGHT},
        {"KEY_A", KEY_A}, {"KEY_B", KEY_B}, {"KEY_C", KEY_C}, {"KEY_D", KEY_D}, {"KEY_E", KEY_E},
        {"KEY_F", KEY_F}, {"KEY_G", KEY_G}, {"KEY_H", KEY_H}, {"KEY_I", KEY_I}, {"KEY_J", KEY_J},
        {"KEY_K", KEY_K}, {"KEY_L", KEY_L}, {"KEY_M", KEY_M}, {"KEY_N", KEY_N}, {"KEY_O", KEY_O},
        {"KEY_P", KEY_P}, {"KEY_Q", KEY_Q}, {"KEY_R", KEY_R}, {"KEY_S", KEY_S}, {"KEY_T", KEY_T},
        {"KEY_U", KEY_U}, {"KEY_V", KEY_V}, {"KEY_W", KEY_W}, {"KEY_X", KEY_X}, {"KEY_Y", KEY_Y},
        {"KEY_Z", KEY_Z},
        {"KEY_SHIFT", KEY_LEFT_SHIFT},
        {"KEY_LEFT_SHIFT", KEY_LEFT_SHIFT},
        {"KEY_RIGHT_SHIFT", KEY_RIGHT_SHIFT},
        {"KEY_CTRL", KEY_LEFT_CONTROL},
    };

    auto it = keyMap.find(keyName);
    return (it != keyMap.end()) ? it->second : KEY_SPACE;
}

Settings LoadSettings(const std::string& filepath) {
    // Settings Object
    Settings settings;

    // Open the file
    std::ifstream file(filepath);
    if (!file.is_open()) {
        std::cerr << "Failed to open settings file: " << filepath << std::endl;
        return settings;
    }

    // Parse the JSON
    json json_root = json::parse(file);
    if (!json_root.contains("tilemap")) {
        std::cerr << "Missing tilemap in settings file: " << filepath << std::endl;
        return settings;
    }

    if (!LoadTilemapSettings(json_root["tilemap"], settings.tilemap)) {
        // return early if there is an error with the load tilemap settings function
        return settings;
    }

    // Load keybinds
    if (json_root.contains("keybinds")) {
        json json_keybinds = json_root["keybinds"];
        if (json_keybinds.contains("jump")) settings.keybinds.jump = StringToKeyboardKey(json_keybinds["jump"].get<std::string>());
        if (json_keybinds.contains("dash")) settings.keybinds.dash = StringToKeyboardKey(json_keybinds["dash"].get<std::string>());
        if (json_keybinds.contains("grab")) settings.keybinds.grab = StringToKeyboardKey(json_keybinds["grab"].get<std::string>());
        if (json_keybinds.contains("up")) settings.keybinds.up = StringToKeyboardKey(json_keybinds["up"].get<std::string>());
        if (json_keybinds.contains("down")) settings.keybinds.down = StringToKeyboardKey(json_keybinds["down"].get<std::string>());
        if (json_keybinds.contains("left")) settings.keybinds.left = StringToKeyboardKey(json_keybinds["left"].get<std::string>());
        if (json_keybinds.contains("right")) settings.keybinds.right = StringToKeyboardKey(json_keybinds["right"].get<std::string>());
    }

    // Load sounds
    if (json_root.contains("sounds")) {
        json json_sounds = json_root["sounds"];
        if (json_sounds.contains("jump")) settings.sounds.jump = json_sounds["jump"].get<std::string>();
        if (json_sounds.contains("dash")) settings.sounds.dash = json_sounds["dash"].get<std::string>();
        if (json_sounds.contains("death")) settings.sounds.death = json_sounds["death"].get<std::string>();
        if (json_sounds.contains("bgm")) settings.sounds.bgm = json_sounds["bgm"].get<std::string>();
    }

    // Load player animations
    if (json_root.contains("player_animations")) {
        json json_player_animations = json_root["player_animations"];
        if (json_player_animations.contains("idle")) settings.player_animations.idle = json_player_animations["idle"].get<std::string>();
        if (json_player_animations.contains("run")) settings.player_animations.run = json_player_animations["run"].get<std::string>();
        if (json_player_animations.contains("jump")) settings.player_animations.jump = json_player_animations["jump"].get<std::string>();
        if (json_player_animations.contains("fall")) settings.player_animations.fall = json_player_animations["fall"].get<std::string>();
        if (json_player_animations.contains("fall_pose")) settings.player_animations.fall_pose = json_player_animations["fall_pose"].get<std::string>();
        if (json_player_animations.contains("dash")) settings.player_animations.dash = json_player_animations["dash"].get<std::string>();
        if (json_player_animations.contains("death")) settings.player_animations.death = json_player_animations["death"].get<std::string>();
    }

    return settings;
}