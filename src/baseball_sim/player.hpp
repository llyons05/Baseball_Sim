#pragma once

#include "statistics.hpp"
#include "includes.hpp"

#include <string>
#include <map>
#include <unordered_map>
#include <memory>


const std::map<std::string, std::string> POSITION_TO_APPEARANCE_KEY = {{"pitcher", "games_at_p"}, {"catcher", "games_at_c"}, {"1B", "games_at_1b"}, {"2B", "games_at_2b"}, {"3B", "games_at_3b"}, {"SS", "games_at_ss"}, {"LF", "games_at_lf"}, {"CF", "games_at_cf"}, {"RF", "games_at_rf"}, {"DH", "games_at_dh"}};


class Player {
    
    public:
        std::string name, id;
        Player_Stats stats;
        unsigned int day_of_last_game_played;

        Player() {}

        Player(const std::string& name, const Player_Stats& stats) {
            this->stats = stats;
            this->name = name;
            id = stats.player_id;
            day_of_last_game_played = 1000;
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