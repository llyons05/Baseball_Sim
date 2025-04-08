#pragma once

#include "player.hpp"
#include "statistics.hpp"

#include <string>
#include <vector>
#include <algorithm>
#include <set>
#include <memory>

enum eTeam {
    AWAY_TEAM,
    HOME_TEAM
};

class Team {
    public:
        std::string team_name;
        Team_Stats team_stats;

        std::vector<Player*> all_players;
        Player* batting_order[9];
        Player* fielders[NUM_DEFENSIVE_POSITIONS];
        std::set<Player*> available_pitchers;

        int position_in_batting_order;
        int runs_allowed_by_pitcher;

        Team(){}

        Team(const std::string& team_name, const std::vector<Player*>& players, const Team_Stats& team_stats);

        void add_player(Player* player) {
            all_players.push_back(player);
        }

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

        Player* try_switching_pitcher(int current_half_inning);
        Player* pick_next_pitcher(int current_half_inning);
        void set_current_pitcher(Player* new_pitcher);
        void set_position_in_field(Player* new_player, eDefensivePositions position);

        void print_fielders();
        void print_batting_order();

        void reset();

    private:

        void set_up_batting_order();
        void set_up_fielders();
        void set_up_pitchers();

        Player* pick_starting_pitcher();
        Player* pick_relief_pitcher();
        std::set<Player*> get_all_pitchers();

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


extern std::unordered_map<std::string, std::shared_ptr<Team>> team_cache;