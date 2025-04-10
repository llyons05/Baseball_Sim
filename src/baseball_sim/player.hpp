#pragma once

#include "statistics.hpp"

#include <string>
#include <map>
#include <unordered_map>
#include <memory>

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
    POS_DH,
    NUM_DEFENSIVE_POSITIONS,
    POS_NONE
};


const std::string DEFENSIVE_POSITIONS[NUM_DEFENSIVE_POSITIONS] = {"pitcher", "catcher", "1B", "2B", "3B", "SS", "LF", "CF", "RF", "DH"};
const std::map<std::string, std::string> POSITION_TO_APPEARANCE_KEY = {{"pitcher", "games_at_p"}, {"catcher", "games_at_c"}, {"1B", "games_at_1b"}, {"2B", "games_at_2b"}, {"3B", "games_at_3b"}, {"SS", "games_at_ss"}, {"LF", "games_at_lf"}, {"CF", "games_at_cf"}, {"RF", "games_at_rf"}, {"DH", "games_at_dh"}};


class Player {
    
    public:
        std::string name, id;
        Player_Stats stats;

        Player() {}

        Player(const std::string& name, const Player_Stats& stats) {
            this->stats = stats;
            this->name = name;
            id = stats.player_id;
        }

        int games_at_fielding_position(eDefensivePositions position) const {
            std::string stat_string = POSITION_TO_APPEARANCE_KEY.at(DEFENSIVE_POSITIONS[position]);
            return stats.get_stat(PLAYER_APPEARANCES, stat_string, .0f);
        }

        bool operator<(const Player& other) const {
            return id < other.id;
        }

        bool operator==(const Player& other) const {
            return id == other.id;
        }
};


extern std::unordered_map<std::string, std::unique_ptr<Player>> player_cache;