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
    if(uiLibrary.Button(0, "NEW GAME", {100, 325, 300, 50})) {
        if (GetSceneManager() != nullptr) {
            GetSceneManager()->use_saved_checkpoint = false;
            GetSceneManager()->SwitchScene(1);
        }   
    }
    if(uiLibrary.Button(1, "CONTINUE GAME", {100, 375, 300, 50})) {
        if (GetSceneManager() != nullptr) {
            GetSceneManager()->use_saved_checkpoint = true;
            GetSceneManager()->SwitchScene(1);
        }
    }
    if(uiLibrary.Button(2, "LEADERBOARD", {100, 425, 300, 50})) {
        if (GetSceneManager() != nullptr) {
            GetSceneManager()->SwitchScene(3);
        }
    }
    if(uiLibrary.Button(3, "EXIT GAME", {100, 475, 300, 50})) {
        CloseWindow();
    }
}
