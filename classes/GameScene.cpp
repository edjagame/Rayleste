#include "GameScene.hpp"

GameScene::GameScene() 
    : player({0.0f, 0.0f}, PLAYER_WIDTH, PLAYER_HEIGHT, PLAYER_SPEED, PLAYER_MASS),
      grid(),
      cam_type(0),
      camera_view({0}),
      player_center({0.0f, 0.0f}),
      current_screen_index(-1),
      camera_target_prev({0.0f, 0.0f}),
      camera_target_next({0.0f, 0.0f}),
      screen_transition_timer(0.0f),
      camera_zoom_prev(1.0f),
      camera_zoom_next(1.0f),
      game_state(GAMEPLAY) {
}

void GameScene::Begin() {
    // Init Settings
    settings = LoadSettings(SETTINGS_FILEPATH);
    
    // Load sounds and music via shared resource manager
    sound_jump = ResourceManager::GetInstance()->GetSound(settings.sounds.jump);
    sound_dash = ResourceManager::GetInstance()->GetSound(settings.sounds.dash);
    sound_death = ResourceManager::GetInstance()->GetSound(settings.sounds.death);
    music_bgm = ResourceManager::GetInstance()->GetMusic(settings.sounds.bgm);
    
    // Init Tile Texture via shared resource manager
    tilemap = ResourceManager::GetInstance()->GetTexture(settings.tilemap.image_filename);

    // Init Grid
    grid = Grid(   tilemap, 
                        settings.tilemap.tiles, 
                        settings.tilemap.tile_size_sprite_sheet, 
                        settings.tilemap.tile_size_grid, 
                        settings.tilemap.screens);

    const Vector2 spawn = settings.tilemap.screens[0].checkpoint_position;
    player.position = spawn;
    player.velocity = {0.0f, 0.0f};
    player.acceleration = {0.0f, 0.0f};
    player.current_respawn_point = spawn;
    player.dash_time = 0.0f;
    player.dash_cooldown_timer = 0.0f;
    player.has_dashed = false;
    player.dash_direction = {0.0f, 0.0f};
    player.can_wall_grab = true;
    player.wall_grab_timer = 0.0f;
    player.wall_jump_cooldown_timer = 0.0f;
    player.respawn_timer = 0.0f;
    player.SetState(&player.airborne);
    player.LoadKeybinds(settings.keybinds.jump, settings.keybinds.dash, settings.keybinds.grab,
                        settings.keybinds.up, settings.keybinds.down,
                        settings.keybinds.left, settings.keybinds.right);

    if (!player.LoadAnimations(
        settings.player_animations.idle,
        settings.player_animations.run,
        settings.player_animations.jump,
        settings.player_animations.fall,
        settings.player_animations.fall_pose,
        settings.player_animations.dash,
        settings.player_animations.death
    )) {
        std::cerr << "Player animations failed to load. Using fallback rectangle." << std::endl;
    }
    
    player.sound_jump = &sound_jump;
    player.sound_dash = &sound_dash;
    player.sound_death = &sound_death;
    
    // Play background music (loop)
    SetMusicVolume(music_bgm, 0.2f);
    PlayMusicStream(music_bgm);

    // Init Camera
    cam_type = 0;
    camera_view = {0};

    camera_view.target = SCREEN_CENTER;
    camera_view.offset = {WINDOW_WIDTH/2.0f, WINDOW_HEIGHT/2.0f};
    camera_view.zoom = 1.0f;

    // Init Background via shared resource manager
    background = ResourceManager::GetInstance()->GetTexture(settings.textures.background);
    SetTextureWrap(background, TEXTURE_WRAP_REPEAT);

    //Set references
    player.SetGrid(&grid);

    player_center = {
        player.position.x + player.width / 2.0f,
        player.position.y + player.height / 2.0f
    };

    // Set initial camera position to the screen the player starts on
    current_screen_index = grid.GetScreenIndex(player_center);
    if (current_screen_index >= 0) {
        camera_view.target = grid.GetScreenCenter(current_screen_index);
        player.current_respawn_point = grid.GetScreenCheckpoint(current_screen_index);
    }

    camera_target_prev = camera_view.target;
    camera_target_next = camera_view.target;
    screen_transition_timer = 0.0f;
    camera_zoom_prev = camera_view.zoom;
    camera_zoom_next = camera_view.zoom;
    death_count = 0;
    elapsed_time_seconds = 0.0f;
    player.death_counter = &death_count;

    GetSceneManager()->SetRunStats(0, 0.0f);

    game_state = GAMEPLAY;
}

void GameScene::End() {
    player.UnloadAnimations();
    StopMusicStream(music_bgm);
}

void GameScene::Update() {
    float deltaTime = GetFrameTime();
    // ========== GAME UPDATE ==========

    // Return to title scene
    if (IsKeyPressed(KEY_BACKSPACE)) {
        if (GetSceneManager() != nullptr) {
            GetSceneManager()->SwitchScene(0);
        }
        return;
    }

    // Update background music
    UpdateMusicStream(music_bgm);
    elapsed_time_seconds += deltaTime;

    if (game_state == GAMEPLAY) {
        player.Update(deltaTime);

        if (grid.GetTileType(player.GetCenterPosition()) == TileType::WIN_CRYSTAL) {
            if (GetSceneManager() != nullptr) {
                GetSceneManager()->SetRunStats(death_count, elapsed_time_seconds);
                GetSceneManager()->SwitchScene(2);
            }
            return;
        }

        // updates the camera to follow the player if they move to a new screen
        player_center = {
            player.position.x + player.width / 2.0f,
            player.position.y + player.height / 2.0f
        };
        int new_screen_index = grid.GetScreenIndex(player_center);
        if (new_screen_index >= 0 && new_screen_index != current_screen_index) {
            grid.ResetTiles();
            // get new screen center
            current_screen_index = new_screen_index;
            player.current_respawn_point = grid.GetScreenCheckpoint(current_screen_index);
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
            // when jumping up to a new screen give the player a lil boost
            if (camera_target_next.y < camera_target_prev.y) {
                player.SetState(&player.airborne);
                player.velocity.y = -1000.0f;
            }
            game_state = GAMEPLAY;
        }
    }
}

void GameScene::Draw() {
    Rectangle bg_source = { camera_view.target.x - WINDOW_WIDTH/2.0f, camera_view.target.y - WINDOW_HEIGHT/2.0f, (float)WINDOW_WIDTH, (float)WINDOW_HEIGHT };
    Rectangle bg_dest   = { camera_view.target.x - WINDOW_WIDTH/2.0f, camera_view.target.y - WINDOW_HEIGHT/2.0f, (float)WINDOW_WIDTH, (float)WINDOW_HEIGHT };

    BeginMode2D(camera_view);
    DrawTexturePro(background, bg_source, bg_dest, {0,0}, 0, WHITE);
    
    grid.DrawGrid();
    player.Draw();
    EndMode2D();
}
