#include "WinScene.hpp"
#include <cmath>

WinScene::WinScene() {
}

void WinScene::Begin() {
    settings = LoadSettings("settings.ini");
    music_win = ResourceManager::GetInstance()->GetMusic(settings.sounds.win_music);
    rainbow_accumulator = 0.0f;
    SetMusicVolume(music_win, 0.2f);
    PlayMusicStream(music_win);
}

void WinScene::End() {
    StopMusicStream(music_win);
}

void WinScene::Update() {
    UpdateMusicStream(music_win);
    rainbow_accumulator += GetFrameTime();
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
}
