#pragma once

#include "team.hpp"
#include "game_states.hpp"

#include <string>

const int MAX_HALF_INNINGS = 18;

class Baseball_Game {
    public:
        unsigned int day_of_year;
        int half_inning_count;
        int team_batting;
        int score[2];
        Team* teams[2];

        Baseball_Game(Team* home_team, Team* away_team, unsigned int day_of_year);

        Game_Result play_game();
        void print_game_result();
    
    private:
        int play_half_inning();
};