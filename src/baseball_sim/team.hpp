#pragma once

#include "player.hpp"

enum eTeam {
    AWAY_TEAM,
    HOME_TEAM
};

class Team {
    Player roster[64];
    Pitcher bullpen[32];
    Batter batting_order[9];
    Player fielders[9];
    int position_in_batting_order;
};