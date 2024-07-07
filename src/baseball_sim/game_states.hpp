#pragma once

#include "player.hpp"
#include "team.hpp"

#define FIRST_BASE 0
#define SECOND_BASE 1
#define THIRD_BASE 2

class At_Bat {
    int balls;
    int strikes;

    Pitcher pitcher;
    Batter batter;

    Team pitching_team;
    Team hitting_team;
};

class Base_State {
    Player players_on_base[3];
};

class Half_Inning {
    Team hitting_team;
    Team pitching_team;

    Pitcher pitcher;
    Batter batter;

    int outs;
    int runs_scored;
    Base_State bases;

    Half_Inning();

};

class Game_Result {
    Team teams[2];
    int final_score[2];
    int half_innings_played;
    
};