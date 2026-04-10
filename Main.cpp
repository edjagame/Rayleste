#include <raylib.h>
#include <raymath.h>

#include <iostream>
#include <vector>

#include "classes/Player.cpp"
#include "classes/Settings.cpp"
#include "classes/Grid.cpp"


const int WINDOW_WIDTH = 1280;
const int WINDOW_HEIGHT = 720;
const Vector2 center = {WINDOW_WIDTH/2, WINDOW_HEIGHT/2};
const int FPS = 60;

const std::string SETTINGS_FILEPATH = "settings.ini";

int main() {
    // Init Scene
    InitWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Rayleste");
    SetTargetFPS(FPS);


    // Init Settings
    Settings settings = LoadSettings(SETTINGS_FILEPATH);
    
    // Init Tile Texture
    Texture2D tilemap = LoadTexture(settings.tilemap.image_filename.c_str());

    // Init Grid
    Grid grid = Grid(   tilemap, 
                        settings.tilemap.tiles, 
                        settings.tilemap.tile_size_sprite_sheet, 
                        settings.tilemap.tile_size_grid, 
                        settings.tilemap.grid_coords, 
                        settings.tilemap.grid_data);


    // Init Player
    Player player = Player(center, 50.0f, 50.0f, 300.0f, 1.0f);
    player.SetState(&player.airborne);
    player.LoadKeybinds(settings.keybinds.jump, settings.keybinds.dash, 
                        settings.keybinds.up, settings.keybinds.down,
                        settings.keybinds.left, settings.keybinds.right);

    // Init Camera
    int cam_type;
    Camera2D camera_view = {0};

    camera_view.target = center;
    camera_view.offset = {WINDOW_WIDTH/2.0f, WINDOW_HEIGHT/2.0f};
    camera_view.zoom = 1.0f;

    //Init Background
    Texture2D background = LoadTexture("assets/bg.png");
    SetTextureWrap(background, TEXTURE_WRAP_REPEAT);

    //Set references
    player.SetGrid(&grid);

    while(!WindowShouldClose()){
        float deltaTime = GetFrameTime();
        // ========== UPDATE ==========

        player.Update(deltaTime);
        camera_view.target = player.position;

        // ========== DRAW ==========
        Rectangle bg_source = { camera_view.target.x - WINDOW_WIDTH/2.0f, camera_view.target.y - WINDOW_HEIGHT/2.0f, (float)WINDOW_WIDTH, (float)WINDOW_HEIGHT };
        Rectangle bg_dest   = { camera_view.target.x - WINDOW_WIDTH/2.0f, camera_view.target.y - WINDOW_HEIGHT/2.0f, (float)WINDOW_WIDTH, (float)WINDOW_HEIGHT };
        
        BeginDrawing();
        ClearBackground(BLACK);
        BeginMode2D(camera_view);
        DrawTexturePro(background, bg_source, bg_dest, {0,0}, 0, WHITE);
        
        grid.DrawGrid();
        player.Draw();
        EndMode2D();

    

        EndDrawing();
    }

    UnloadTexture(tilemap);
    UnloadTexture(background);
    CloseAudioDevice();
    CloseWindow();

    return 0;
}