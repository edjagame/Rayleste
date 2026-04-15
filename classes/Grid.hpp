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

class Grid {
public:
    std::vector<Tile> tiles;
    Vector2 coords;       // number of columns (x) and rows (y)
    Vector2 tile_size_grid;   // rendered size of each tile in pixels
    Vector2 tile_size_spritesheet; // pixel size of each tile in the spritesheet
    std::vector<int> grid_data;
    Texture2D tilemap;
    Vector2 tilemap_coords;

    Grid() {}
    Grid(   Texture2D tilemap, std::vector<Tile> tiles, Vector2 tile_size_spritesheet, 
            Vector2 tile_size_grid, Vector2 coords, std::vector<int> grid_data) {
        this->coords = coords;
        this->tile_size_grid = tile_size_grid;
        this->grid_data = grid_data;
        this->tilemap = tilemap;
        this->tiles = tiles;
        this->tile_size_spritesheet = tile_size_spritesheet;
        
        InitTileSize();
    }

    void InitTileSize();
    void DrawGrid();
    TileType GetTileType(Vector2 position);
};
