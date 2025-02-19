#pragma once

#include "player.hpp"
#include "statistics.hpp"

#include <string>
#include <vector>
#include <algorithm>
#include <set>

enum eTeam {
    AWAY_TEAM,
    HOME_TEAM
};

class Team {
    public:
        std::string team_name;
        Team_Stats team_stats;

        std::vector<Player> all_players;
        Player batting_order[9];
        Player fielders[NUM_DEFENSIVE_POSITIONS];

        std::set<Player> available_pitchers;
        int position_in_batting_order;
        int runs_allowed_by_pitcher;

        Team(){}

        Team(const std::string& team_name, const std::vector<Player>& players, const Team_Stats& team_stats);

        void add_player(const Player& player) {
            all_players.push_back(player);
        }

        std::vector<Player> get_players() {
            return all_players;
        }

        inline Player* get_batter() {
            return &batting_order[position_in_batting_order];
        }

        inline Player* get_pitcher() {
            return &fielders[POS_PITCHER];
        }


        std::set<Player> filter_players_by_listed_pos(const std::vector<std::string>& positions = {});
        std::set<Player> filter_pitchers(const std::vector<std::string>& positions = {});

        Player* try_switching_pitcher(int current_half_inning);
        Player* pick_next_pitcher(int current_half_inning);
        void set_current_pitcher(const Player& new_pitcher);
        void set_position_in_field(const Player& new_player, eDefensivePositions position);

        void print_fielders();
        void print_batting_order();

        void reset();

    private:
        Player* pick_starting_pitcher();
        Player* pick_relief_pitcher();

        void set_up_batting_order();
        void set_up_fielders();
        void set_up_pitchers();

        std::set<Player> get_all_pitchers();

        std::set<Player> find_players(const std::vector<std::string>& player_ids) {
            std::set<Player> result;
            for (const Player& player : all_players) {
                for (std::string player_id : player_ids) {
                    if (player.id == player_id) {
                        result.insert(player);
                        break;
                    }
                }
            }
            return result;
        }

        Player find_best_player_for_defense_pos(eDefensivePositions position, const std::vector<Player>& players_to_exclude = {});
};