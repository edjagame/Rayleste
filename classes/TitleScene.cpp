#include "TitleScene.hpp"

void TitleScene::Begin() {
    // Load Sound
    music_bgm = LoadMusicStream("assets/sounds/title_bgm.mp3");
    
    // Play background music (loop)
    SetMusicVolume(music_bgm, 0.2f);
    PlayMusicStream(music_bgm);

    //Init Background
    rayleste_bg = LoadTexture("assets/rayleste-bg.png");
    SetTextureWrap(rayleste_bg, TEXTURE_WRAP_REPEAT);
}

void TitleScene::End() {
    UnloadTexture(rayleste_bg);
    UnloadMusicStream(music_bgm);
}

void TitleScene::Update() {
    // Update background music
    UpdateMusicStream(music_bgm);
    if (IsKeyPressed(KEY_ENTER)) {
        if (GetSceneManager() != nullptr) {
            GetSceneManager()->SwitchScene(1);
        }
    }
}

void TitleScene::Draw() {
    DrawTexture(rayleste_bg, 0, 0 , WHITE);
    DrawText("New Game", 100, 325, 30, WHITE);
    DrawText("Continue Game", 100, 375, 30, WHITE);
    DrawText("Exit Game", 100, 425, 30, WHITE);
}
