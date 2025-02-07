#pragma once

#include "statistics.hpp"

#include <string>
#include <map>


enum eDefensivePositions {
    POS_PITCHER,
    POS_CATCHER,
    POS_1B,
    POS_2B,
    POS_3B,
    POS_SS,
    POS_LF,
    POS_CF,
    POS_RF,
    POS_OF,
    NUM_DEFENSIVE_POSITIONS,
    POS_NONE
};


const std::string DEFENSIVE_POSITIONS[NUM_DEFENSIVE_POSITIONS] = {"pitcher", "catcher", "1B", "2B", "3B", "SS", "LF", "CF", "RF", "OF"};
const std::map<std::string, std::string> POSITION_TO_APPEARANCE_KEY = {{"pitcher", "G_p_app"}, {"catcher", "G_c"}, {"1B", "G_1b"}, {"2B", "G_2b"}, {"3B", "G_3b"}, {"SS", "G_ss"}, {"LF", "G_lf_app"}, {"CF", "G_cf_app"}, {"RF", "G_rf_app"}, {"OF", "G_of_app"}};


class Player {
    
    public:
        std::string name, id;
        Player_Stats stats;

        Player() {}

        Player(std::string name, Player_Stats stats) {
            this->stats = stats;
            this->name = name;
            id = stats.player_id;
        }

        int games_at_fielding_position(eDefensivePositions position) {
            std::string stat_string = POSITION_TO_APPEARANCE_KEY.at(DEFENSIVE_POSITIONS[position]);
            return stats.get_stat(PLAYER_APPEARANCES, stat_string, 0);
        }

        bool operator<(const Player& other) const {
            return id < other.id;
        }

        bool operator==(const Player& other) const {
            return id == other.id;
        }
};