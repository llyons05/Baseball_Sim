#pragma once

#include "player.hpp"

class Player_Stats {
    // Batter Stats
    float batting_avg;
    float obps;
    
    // Pitcher Stats
    float opponent_batting_avg;
    float strikout_percentage;
    float walk_percentage;

};

class Team_Info {
    int wins;
    int losses;

    Player usual_batting_order[9];
};