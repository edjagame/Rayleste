#pragma once

#include "Settings.hpp"
#include "SceneManager.hpp"
#include "immediate.cpp"

class TitleScene : public Scene {

    public:
        Settings settings;
        Music music_bgm;
        Texture2D rayleste_bg;
        // new
        UiLibrary uiLibrary;

        void Begin() override;
        void End() override;
        void Update() override;
        void Draw() override;
};