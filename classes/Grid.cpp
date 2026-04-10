#include "Grid.hpp"

void Grid::InitTileSize() {
    for (Tile& tile : tiles) {
        tile.size = tile_size_spritesheet;
    }
}

void Grid::DrawGrid() {
    for (int row = 0; row < coords.y; row++) {
        for (int col = 0; col < coords.x; col++) {
            int tileId = grid_data[row * coords.x + col];
            for (const Tile& tile : tiles) {
                if (tile.id == tileId) {
                    Rectangle source = { tile.position.x, tile.position.y, tile.size.x, tile.size.y };
                    Rectangle dest   = { col * tile_size_grid.x, row * tile_size_grid.y, tile_size_grid.x, tile_size_grid.y };
                    DrawTexturePro(tilemap, source, dest, {0, 0}, 0.0f, WHITE);
                    break;
                }
            }
        }
    }
}

bool Grid::IsSolidTile(Vector2 position) {
    // convert to grid coords
    int col = position.x / tile_size_grid.x;
    int row = position.y / tile_size_grid.y;


    // check bounds
    if (col < 0 || col >= coords.x || row < 0 || row >= coords.y) {
        return false; 
    }
    
    int tileId = grid_data[row * coords.x + col];
    for (const Tile& tile : tiles) {
        if (tile.id == tileId) {
            return tile.isSolid;
        }
    }
    return false; // Default to non-solid if tile ID not found
}
