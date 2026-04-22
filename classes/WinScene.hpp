#pragma once

#include "SceneManager.hpp"
#include "Settings.hpp"

class WinScene : public Scene {
public:

    Settings settings;
    Music music_win;

    float rainbow_accumulator = 0.0f;

    WinScene ();
    
    void Begin() override;
    void End() override;
    void Update() override;
    void Draw() override;
};
