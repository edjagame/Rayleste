#pragma once

#include "Settings.hpp"
#include "SceneManager.hpp"

class TitleScene : public Scene {

    public:
        Settings settings;
        Music music_bgm;
        Texture2D rayleste_bg;

        void Begin() override;
        void End() override;
        void Update() override;
        void Draw() override;
};