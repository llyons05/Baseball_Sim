#pragma once

#include "utils.hpp"
#include "player.hpp"
#include "statistics.hpp"

#include <string>
#include <vector>
#include <algorithm>
#include <set>
#include <memory>
#include <cstdint>


class Team {
    public:
        std::string team_name;
        Team_Stats team_stats;

        std::vector<Player*> all_players;
        Player* batting_order[9];
        Player* fielders[NUM_DEFENSIVE_POSITIONS];
        std::set<Player*> available_pitchers;
        std::vector<Player*> pitchers_used;

        uint8_t position_in_batting_order;
        uint8_t runs_allowed_by_pitcher;
        uint8_t current_pitcher_starting_half_inning;

        Team_Running_Stat_Container running_stats;

        Team(){}
        Team(const std::string& team_name, const std::vector<Player*>& players, const Team_Stats& team_stats);

        const std::vector<Player*>& get_players() {
            return all_players;
        }

        inline Player* get_batter() {
            return batting_order[position_in_batting_order];
        }

        inline Player* get_pitcher() {
            return fielders[POS_PITCHER];
        }


        std::set<Player*> filter_players_by_listed_pos(const std::vector<Table_Entry>& positions = {});
        std::set<Player*> filter_pitchers(const std::vector<Table_Entry>& positions = {});

        Player* try_switching_pitcher(uint8_t current_half_inning, uint current_day_of_year);
        Player* pick_next_pitcher(uint8_t current_half_inning, uint current_day_of_year);
        void set_current_pitcher(Player* new_pitcher, uint8_t current_half_inning);
        void set_position_in_field(Player* new_player, eDefensivePositions position);

        void print_fielders();
        void print_batting_order();

        void prepare_for_game(uint day_of_game, bool keep_batting_order);
        void reset_player_tracking_data();

    private:
        const uint MAX_PITCHER_COOLDOWN = 15; // days

        void set_up_batting_order();
        void set_up_fielders();
        void set_up_pitchers();

        Player* pick_starting_pitcher(uint current_day_of_year);
        Player* pick_relief_pitcher(uint current_day_of_year);
        std::set<Player*> get_all_pitchers();
        bool should_swap_pitcher(Player* pitcher, uint8_t current_half_inning);

        std::set<Player*> find_players(const std::vector<std::string>& player_ids) {
            std::set<Player*> result;
            for (const std::string& player_id : player_ids) {
                Player* player = player_cache.at(get_player_cache_id(player_id, team_stats.team_cache_id)).get();
                result.insert(player);
            }
            return result;
        }

        Player* find_best_player_for_defense_pos(eDefensivePositions position, const std::vector<std::string>& players_to_exclude = {});
};


extern std::unordered_map<std::string, std::unique_ptr<Team>> team_cache;