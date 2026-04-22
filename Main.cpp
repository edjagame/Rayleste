#include <raylib.h>
#include <raymath.h>

#include <iostream>
#include <vector>

#include "classes/SceneManager.hpp"
#include "classes/GameScene.hpp"
#include "classes/WinScene.hpp"
#include "classes/Grid.cpp"
#include "classes/Player.cpp"
#include "classes/Settings.cpp"
#include "classes/GameScene.cpp"
#include "classes/WinScene.cpp"

const int FPS = 60;

int main() {
    // Init Scene
    InitWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Rayleste");
    SetTargetFPS(FPS);

    InitAudioDevice();

    SceneManager scene_manager;

    GameScene game_scene;
    WinScene win_scene;
    
    game_scene.SetSceneManager(&scene_manager);
    win_scene.SetSceneManager(&scene_manager);

    scene_manager.RegisterScene(&game_scene, 1);
    scene_manager.RegisterScene(&win_scene, 2);

    scene_manager.SwitchScene(1);

    while(!WindowShouldClose()) {
        Scene* active_scene = scene_manager.GetActiveScene();

        BeginDrawing();
        ClearBackground(BLACK);

        if (active_scene != nullptr) {
            active_scene->Update();
            active_scene->Draw();
        }

        EndDrawing();
    }

    Scene* active_scene = scene_manager.GetActiveScene();
    if (active_scene != nullptr) {
        active_scene->End();
    }

    ResourceManager::GetInstance()->UnloadAllMusic();
    ResourceManager::GetInstance()->UnloadAllSounds();
    ResourceManager::GetInstance()->UnloadAllTextures();

    CloseAudioDevice();
    CloseWindow();

    return 0;
}