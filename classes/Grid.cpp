#include "Grid.hpp"

void Grid::InitTileSize() {
    for (Tile& tile : tiles) {
        tile.size = tile_size_spritesheet;
    }
}

void Grid::DrawGrid() {
    for (const Screen& screen : screens) {
        const int screen_cols = static_cast<int>(screen.dimensions_tiles.x);
        const int screen_rows = static_cast<int>(screen.dimensions_tiles.y);

        for (int row = 0; row < screen_rows; row++) {
            for (int col = 0; col < screen_cols; col++) {
                // converts 2d coordinates for the 1d vector
                const int index = row * screen_cols + col;
                const int tileId = screen.tile_data[index];

                for (const Tile& tile : tiles) {
                    if (tile.id == tileId) {
                        Rectangle source = { tile.position.x, tile.position.y, tile.size.x, tile.size.y };
                        Rectangle dest   = {
                            screen.world_position.x + col * tile_size_grid.x,
                            screen.world_position.y + row * tile_size_grid.y,
                            tile_size_grid.x,
                            tile_size_grid.y
                        };
                        DrawTexturePro(tilemap, source, dest, {0, 0}, 0.0f, WHITE);
                        break;
                    }
                }
            }
        }
    }
}

TileType Grid::GetTileType(Vector2 position) {
    // default to solid on edge cases

    const int screen_index = GetScreenIndex(position);
    if (screen_index < 0) {
        return TileType::SOLID;
    }

    const Screen& screen = screens[screen_index];
    const int screen_cols = screen.dimensions_tiles.x;
    const int screen_rows = screen.dimensions_tiles.y;

    if (screen_cols <= 0 || screen_rows <= 0) {
        return TileType::SOLID;
    }

    const int col = static_cast<int>((position.x - screen.world_position.x) / tile_size_grid.x);
    const int row = static_cast<int>((position.y - screen.world_position.y) / tile_size_grid.y);

    if (col < 0 || col >= screen_cols || row < 0 || row >= screen_rows) {
        return TileType::SOLID;
    }

    const int index = row * screen_cols + col;
    if (index < 0 || index >= screen.tile_data.size()) {
        return TileType::SOLID;
    }

    // if we made it here, we know the tile exists and can check its type
    const int tileId = screen.tile_data[index];
    for (const Tile& tile : tiles) {
        if (tile.id == tileId) {
            return tile.type;
        }
    }
    return TileType::FLOOR;
}

int Grid::GetScreenIndex(Vector2 position) const {
    // Brute force searching through screens
    for (int index = 0; index < screens.size(); index++) {
        const Screen& screen = screens[index];
        const int screen_cols = screen.dimensions_tiles.x;
        const int screen_rows = screen.dimensions_tiles.y;

        // AABB check kinda
        const float left = screen.world_position.x;
        const float top = screen.world_position.y;
        const float right = left + screen_cols * tile_size_grid.x;
        const float bottom = top + screen_rows * tile_size_grid.y;
        if (position.x >= left && position.x < right && position.y >= top && position.y < bottom) {
            return index;
        }
    }

    // should ideally never happen
    return -1;
}

Vector2 Grid::GetScreenCenter(int screen_index) const {
    const Screen& screen = screens[screen_index];
    return {
        screen.world_position.x + (screen.dimensions_tiles.x * tile_size_grid.x) / 2.0f,
        screen.world_position.y + (screen.dimensions_tiles.y * tile_size_grid.y) / 2.0f
    };
}

Vector2 Grid::GetScreenDimensions(int screen_index) const {
    if (screen_index < 0 || screen_index >= screens.size()) {
        return {0.0f, 0.0f};
    }
    const Screen& screen = screens[screen_index];
    return screen.dimensions_tiles;
}

Vector2 Grid::GetScreenCheckpoint(int screen_index) const {
    if (screen_index < 0 || screen_index >= screens.size()) {
        return {0.0f, 0.0f};
    }
    const Screen& screen = screens[screen_index];
    return screen.checkpoint_position;
}
