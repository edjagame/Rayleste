#pragma once

#include "SceneManager.hpp"
#include "Settings.hpp"
#include "immediate.cpp"

class WinScene : public Scene {
public:

    Settings settings;
    Music music_win;
    UiLibrary uiLibrary;

    float rainbow_accumulator = 0.0f;
    int final_death_count = 0;
    float final_time_seconds = 0.0f;

    WinScene ();
    
    void Begin() override;
    void End() override;
    void Update() override;
    void Draw() override;
};
