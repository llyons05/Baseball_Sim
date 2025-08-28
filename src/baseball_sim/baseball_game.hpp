#pragma once

#include "team.hpp"
#include "game_states.hpp"

#include <string>

const int MAX_HALF_INNINGS = 18;

class Baseball_Game {
    public:
        uint day_of_year;
        uint8_t half_inning_count;
        uint8_t team_batting;
        int score[2];
        Team* teams[2];

        Baseball_Game(Team* home_team, Team* away_team, uint day_of_year);

        Game_Result play_game();
        void print_game_result();
    
    private:
        uint8_t play_half_inning();
        float get_runs_to_end_game();
};