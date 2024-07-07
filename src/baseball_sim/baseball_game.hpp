#pragma once

#include "team.hpp"
#include "game_states.hpp"

#include <string>

const int MAX_HALF_INNINGS = 17;

class Baseball_Game {
    int half_inning;
    int team_batting;
    int score[2];
    Team teams[2];

    Baseball_Game( Team home_team, Team away_team );

    Game_Result play_game();
    int play_half_inning();
};