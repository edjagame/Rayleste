#pragma once

#include "Player.hpp"
#include "Settings.hpp"
#include "Grid.hpp"
#include "SceneManager.hpp"

const int WINDOW_WIDTH = 1280;
const int WINDOW_HEIGHT = 720;
const Vector2 SCREEN_CENTER = {WINDOW_WIDTH / 2.0f, WINDOW_HEIGHT / 2.0f};
const float SCREEN_TRANSITION_DURATION = 0.5f;

const std::string SETTINGS_FILEPATH = "settings.ini";

const float PLAYER_WIDTH = 40.0f;
const float PLAYER_HEIGHT = 80.0f;
const float PLAYER_SPEED = 400.0f;
const float PLAYER_MASS = 1.0f;

enum GameState {
    GAMEPLAY,
    TRANSITION,
};

class GameScene : public Scene {
    public:
        Settings settings;

        Sound sound_jump;
        Sound sound_dash;
        Sound sound_death;
        Music music_bgm;

        Texture2D tilemap;
        Texture2D background;
        Grid grid;

        Player player;

        int cam_type;
        Camera2D camera_view;
        Vector2 player_center;
        int current_screen_index = -1;
        Vector2 camera_target_prev;
        Vector2 camera_target_next;
        float screen_transition_timer;
        float camera_zoom_prev;
        float camera_zoom_next;
        int death_count = 0;
        float elapsed_time_seconds = 0.0f;

        GameState game_state;

    public:
        GameScene();

        void Begin() override;
        void End() override;
        void Update() override;
        void Draw() override;
};