#pragma once

#include <raylib.h>

#include <vector>

enum TileType {
    FLOOR = 0,
    SOLID = 1,
    SPIKE = 2,
};

struct Tile {
    int id;
    Vector2 position;
    TileType type;
    Vector2 size;
};

struct Screen {
    Vector2 world_position;
    Vector2 dimensions_tiles;
    std::vector<int> tile_data;
};

class Grid {
public:
    std::vector<Tile> tiles;
    std::vector<Screen> screens;
    Vector2 tile_size_grid;   // rendered size of each tile in pixels
    Vector2 tile_size_spritesheet; // pixel size of each tile in the spritesheet
    Texture2D tilemap;
    Vector2 tilemap_coords;

    Grid() {}
    Grid(   Texture2D tilemap, std::vector<Tile> tiles, Vector2 tile_size_spritesheet,
            Vector2 tile_size_grid, const std::vector<Screen>& screens) {
        this->tile_size_grid = tile_size_grid;
        this->tilemap = tilemap;
        this->tiles = tiles;
        this->tile_size_spritesheet = tile_size_spritesheet;
        this->screens = screens;

        InitTileSize();
    }

    void InitTileSize();
    void DrawGrid();
    TileType GetTileType(Vector2 position);
    int GetScreenIndex(Vector2 position) const;
    Vector2 GetScreenCenter(int screen_index) const;
};
