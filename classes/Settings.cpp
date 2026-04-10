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

Color LoadColor(const json& j) {
    Color color;
    color.r = j["r"].get<unsigned char>();
    color.g = j["g"].get<unsigned char>();
    color.b = j["b"].get<unsigned char>();
    color.a = 255;
    return color;
}

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
    json json_tilemap = json_root["tilemap"];

    // Load tile data
    settings.tilemap.image_filename = json_tilemap["image_filename"].get<std::string>();
    settings.tilemap.tile_size_sprite_sheet = LoadVector2(json_tilemap["tile_size_sprite_sheet"]);
    settings.tilemap.tile_size_grid = LoadVector2(json_tilemap["tile_size_grid"]);
    
    json tiles = json_tilemap["tiles"];
    for (json::iterator it = tiles.begin(); it != tiles.end(); ++it) {
        const json& tile = *it;
        int id = tile["id"].get<int>();
        Vector2 pos = LoadVector2(tile["pos"]);
        bool isSolid = tile["isSolid"].get<bool>();
        settings.tilemap.tiles.push_back({id, pos, isSolid});
    }

    // Load grid data
    settings.tilemap.grid_coords = LoadVector2(json_tilemap["grid_coords"]);
    settings.tilemap.grid_data = json_tilemap["grid_data"].get<std::vector<int>>();

    // Load keybinds
    if (json_root.contains("keybinds")) {
        json json_keybinds = json_root["keybinds"];
        if (json_keybinds.contains("jump")) settings.keybinds.jump = StringToKeyboardKey(json_keybinds["jump"].get<std::string>());
        if (json_keybinds.contains("dash")) settings.keybinds.dash = StringToKeyboardKey(json_keybinds["dash"].get<std::string>());
        if (json_keybinds.contains("up")) settings.keybinds.up = StringToKeyboardKey(json_keybinds["up"].get<std::string>());
        if (json_keybinds.contains("down")) settings.keybinds.down = StringToKeyboardKey(json_keybinds["down"].get<std::string>());
        if (json_keybinds.contains("left")) settings.keybinds.left = StringToKeyboardKey(json_keybinds["left"].get<std::string>());
        if (json_keybinds.contains("right")) settings.keybinds.right = StringToKeyboardKey(json_keybinds["right"].get<std::string>());
    }

    return settings;
}