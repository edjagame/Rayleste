#pragma once

#include "SceneManager.hpp"
#include "Settings.hpp"
#include <string>

const std::string LEADERBOARD_FILEPATH = "rayleste_times.txt";

struct ScoreEntries {
    std::string name;
    float time;
};

class LeaderboardScene : public Scene {
public:

    Settings settings;
    Music music_leaderboard;
    std::vector<ScoreEntries> score_entries;
    bool is_inputting_name = false;
    std::string player_name = ""; 
    float new_time = 0.0f;

    LeaderboardScene();
    
    void Begin() override;
    void End() override;
    void Update() override;
    void Draw() override;
    
    std::vector<ScoreEntries> LoadScores(const std::string& filename);
    void SaveScores(const std::string& filename, const std::vector<ScoreEntries>& score_entries);
};
