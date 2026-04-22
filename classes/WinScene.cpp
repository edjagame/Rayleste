#include "WinScene.hpp"
#include <cmath>

WinScene::WinScene() {
}

int GetCenteredTextX(const char* text, int font_size) {
    return (GetScreenWidth() - MeasureText(text, font_size)) / 2;
}

void WinScene::Begin() {
    settings = LoadSettings("settings.ini");
    music_win = ResourceManager::GetInstance()->GetMusic(settings.sounds.win_music);
    rainbow_accumulator = 0.0f;

    final_death_count = 0;
    final_time_seconds = 0.0f;
    final_death_count = GetSceneManager()->GetRunDeaths();
    final_time_seconds = GetSceneManager()->GetRunCompletionTimeSeconds();

    SetMusicVolume(music_win, 0.2f);
    PlayMusicStream(music_win);
}

void WinScene::End() {
    StopMusicStream(music_win);
}

void WinScene::Update() {
    UpdateMusicStream(music_win);
    rainbow_accumulator += GetFrameTime();
    if (IsKeyPressed(KEY_ENTER)) {
        if (GetSceneManager() != nullptr) {
            GetSceneManager()->SwitchScene(3);
        }
    }
}

void WinScene::Draw() {
    Vector2 center = {WINDOW_WIDTH / 2.0f, WINDOW_HEIGHT / 2.0f};
    const float phase = 5.0f * rainbow_accumulator;
    const float phase_offset = 2.0f * PI / 3.0f;

    ClearBackground(BLACK);

    Color rainbow_color = {
        (unsigned char)((sinf(phase) + 1.0f) / 2.0f * 255),
        (unsigned char)((sinf(phase + phase_offset) + 1.0f) / 2.0f * 255),
        (unsigned char)((sinf(phase + 2.0f * phase_offset) + 1.0f) / 2.0f * 255),
        255
    };

    Font font = GetFontDefault();
    const char* win_text = "You Win!";
    float font_size = 84.0f + 50.0f * sinf(phase * 0.5f);
    float spacing = 5.0f;
    float rotation = 360.0f * sinf(phase * 0.35f);
    Vector2 text_size = MeasureTextEx(font, win_text, font_size, spacing);

    DrawTextPro(font,
                win_text,
                center,
                {text_size.x / 2.0f, text_size.y / 2.0f},
                rotation,
                font_size,
                spacing,
                rainbow_color);

    int minutes = static_cast<int>(final_time_seconds) / 60;
    float seconds = fmodf(final_time_seconds, 60.0f);

    const int stats_font_size = 30;
    const char* time_text = TextFormat("Time: %02i:%05.2f", minutes, seconds);
    DrawText(time_text, GetCenteredTextX(time_text, stats_font_size), center.y + 150, stats_font_size, RAYWHITE);
    const char* death_text = TextFormat("Deaths: %i", final_death_count);
    DrawText(death_text, GetCenteredTextX(death_text, stats_font_size), center.y + 200, stats_font_size, RAYWHITE);
}
