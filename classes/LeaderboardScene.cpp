#include "LeaderboardScene.hpp"
#include "WinScene.hpp"
#include <cmath>
#include <vector>
#include <string>
#include <fstream>


LeaderboardScene::LeaderboardScene() {
}

void LeaderboardScene::Begin() {
    settings = LoadSettings("settings.ini");
    music_leaderboard = ResourceManager::GetInstance()->GetMusic(settings.sounds.win_music);

    new_time = GetSceneManager()->GetRunCompletionTimeSeconds();

    if (new_time > 0.0f) {
        is_inputting_name = true;
        player_name = "";
    }
    else 
    {
        score_entries = LoadScores(LEADERBOARD_FILEPATH);
    }

    SetMusicVolume(music_leaderboard, 0.2f);
    PlayMusicStream(music_leaderboard);
}

void LeaderboardScene::End() {
    StopMusicStream(music_leaderboard);
}

void LeaderboardScene::Update() {
    UpdateMusicStream(music_leaderboard);

    if(is_inputting_name) {
        int key = GetCharPressed();
        while (key > 0) {
            if ((key >= 32) && (key <= 126) && player_name.length() < 3) {
                player_name += static_cast<char>(key);
            }
            key = GetCharPressed();
        }

        if(IsKeyPressed(KEY_BACKSPACE) && !player_name.empty()) {
            player_name.pop_back();
        }

        if(IsKeyPressed(KEY_ENTER) && !player_name.empty()) {
            score_entries = LoadScores(LEADERBOARD_FILEPATH);
            score_entries.push_back({player_name, new_time});
            std::sort(score_entries.begin(), score_entries.end(), [](const ScoreEntries& a, const ScoreEntries& b) {
                // Sort by time in ascending order
                return a.time < b.time; 
            });
            if (score_entries.size() > 10) {
                score_entries.resize(10);
            }
            SaveScores(LEADERBOARD_FILEPATH, score_entries);
            is_inputting_name = false;
        }
    }
    else{
        if (IsKeyPressed(KEY_ENTER)) {
            if (GetSceneManager() != nullptr) {
                GetSceneManager()->SwitchScene(0);
            }
        }
    }


}

void LeaderboardScene::Draw() {
    Vector2 center = {WINDOW_WIDTH / 2.0f, WINDOW_HEIGHT / 2.0f - 280.0f};

    ClearBackground(BLACK);


    Font font = GetFontDefault();
    const char* leaderboard_text = "LEADERBOARD";
    float font_size = 84.0f;
    float spacing = 5.0f;
    Vector2 text_size = MeasureTextEx(font, leaderboard_text, font_size, spacing);

    DrawTextPro(font,
                leaderboard_text,
                center,
                {text_size.x / 2.0f, text_size.y / 2.0f},
                0.0f,
                font_size,
                spacing,
                WHITE);

    if(is_inputting_name) {
        const char* input_text = "Enter name:";
        DrawText(input_text, center.x - MeasureText(input_text, 30) / 2.0f, center.y + 50, 30, WHITE);
        DrawText(player_name.c_str(), center.x - MeasureText(player_name.c_str(), 30) / 2.0f, center.y + 100, 30, WHITE);
    }
    else {
        // references: 
        // https://stackoverflow.com/questions/35544064/how-can-you-switch-a-float-to-a-time-format
        // https://cppreference.com/c/numeric/math/fmod
        for (size_t i =0; i < score_entries.size(); ++i) {
            int minutes = static_cast<int>(score_entries[i].time)/60;
            float seconds = fmodf(score_entries[i].time, 60.0f);
            std::string time_text = TextFormat("%02i:%05.2f", minutes, seconds);
            std::string entry = score_entries[i].name + ": " + time_text;
            DrawText(entry.c_str(), center.x - 100, center.y + 100 + i * 40, 30, WHITE);
        }
    }

    if(uiLibrary.Button(0, "TITLE SCREEN", {100, 600, 300, 50})) {
        if (GetSceneManager() != nullptr) {
            GetSceneManager()->SwitchScene(0);
        }   
    }
    if(uiLibrary.Button(1, "NEW GAME", {550, 600, 300, 50})) {
        if (GetSceneManager() != nullptr) {
            GetSceneManager()->SwitchScene(1);
        }
    }
    if(uiLibrary.Button(2, "EXIT GAME", {950, 600, 300, 50})) {
            GetSceneManager()->should_exit = true;
    }
}

std::vector<ScoreEntries> LeaderboardScene::LoadScores(const std::string& filename) {
    // Scores
    std::vector<ScoreEntries> scores;
    std::ifstream file(filename);
    if (!file.is_open()) return scores;

    std::string name;
    float time;

    while (file >> name >> time) {
        scores.push_back({name, time});
    }

    file.close();
    return scores;
}

void LeaderboardScene::SaveScores(const std::string& filename, const std::vector<ScoreEntries>& score_entries) {
    std::ofstream file(filename);
    for (const ScoreEntries& entry : score_entries) {
        file << entry.name << " " << entry.time << "\n";
    }
    file.close();
}


