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
const float SCREEN_TRANSITION_DURATION = 0.5f;

const std::string SETTINGS_FILEPATH = "settings.ini";

const float PLAYER_WIDTH = 50.0f;
const float PLAYER_HEIGHT = 50.0f;
const float PLAYER_SPEED = 400.0f;
const float PLAYER_MASS = 1.0f;

enum GameState {
    GAMEPLAY,
    TRANSITION,
};

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
                        settings.tilemap.screens);


    // Init Player
    Player player = Player(center, PLAYER_WIDTH, PLAYER_HEIGHT, PLAYER_SPEED, PLAYER_MASS);
    player.SetState(&player.airborne);
    player.LoadKeybinds(settings.keybinds.jump, settings.keybinds.dash, settings.keybinds.grab,
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

    Vector2 player_center = {
        player.position.x + player.width / 2.0f,
        player.position.y + player.height / 2.0f
    };

    // Set initial camera position to the screen the player starts on
    int current_screen_index = grid.GetScreenIndex(player_center);
    if (current_screen_index >= 0) {
        camera_view.target = grid.GetScreenCenter(current_screen_index);
    }

    Vector2 camera_target_prev = camera_view.target;
    Vector2 camera_target_next = camera_view.target;
    float screen_transition_timer = 0.0f;
    float camera_zoom_prev = camera_view.zoom;
    float camera_zoom_next = camera_view.zoom;

    GameState game_state = GAMEPLAY;

    while(!WindowShouldClose()){
        float deltaTime = GetFrameTime();
        // ========== GAME UPDATE ==========

        if (game_state == GAMEPLAY) {
            player.Update(deltaTime);

            // updates the camera to follow the player if they move to a new screen
            player_center = {
                player.position.x + player.width / 2.0f,
                player.position.y + player.height / 2.0f
            };
            int new_screen_index = grid.GetScreenIndex(player_center);
            if (new_screen_index >= 0 && new_screen_index != current_screen_index) {
                // get new screen center
                current_screen_index = new_screen_index;
                camera_target_prev = camera_view.target;
                camera_target_next = grid.GetScreenCenter(current_screen_index);
                screen_transition_timer = 0.0f;
                
                // calculate new camera zoom
                camera_zoom_prev = camera_view.zoom;
                Vector2 new_screen_dimensions = grid.GetScreenDimensions(new_screen_index);
                float screen_width = new_screen_dimensions.x * grid.tile_size_grid.x;
                float screen_height = new_screen_dimensions.y * grid.tile_size_grid.y;
                camera_zoom_next = fminf(WINDOW_WIDTH / screen_width, WINDOW_HEIGHT / screen_height);
                
                game_state = TRANSITION;
            }
        }

        else if (game_state == TRANSITION) {
            if (screen_transition_timer < SCREEN_TRANSITION_DURATION) {
                screen_transition_timer += deltaTime;
                float t = screen_transition_timer / SCREEN_TRANSITION_DURATION;
                t = (t > 1.0f) ? 1.0f : t;
                camera_view.target = Vector2Lerp(camera_target_prev, camera_target_next, t);
                camera_view.zoom = Lerp(camera_zoom_prev, camera_zoom_next, t);
            }
            else {
                camera_view.target = camera_target_next;
                camera_view.zoom = camera_zoom_next;
                game_state = GAMEPLAY;
            }
        }

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