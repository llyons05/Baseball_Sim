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

        Team(std::string team_name, std::vector<Player> players, Team_Stats team_stats);

        void add_player(Player player) {
            all_players.push_back(player);
        }

        std::vector<Player> get_players() {
            return all_players;
        }

        Player get_batter() {
            return batting_order[position_in_batting_order];
        }

        Player get_pitcher() {
            return fielders[POS_PITCHER];
        }


        std::set<Player> filter_players_by_listed_pos(std::vector<std::string> positions = {});
        std::set<Player> filter_pitchers(std::vector<std::string> positions = {});

        Player try_switching_pitcher(int current_half_inning);
        Player pick_next_pitcher(int current_half_inning);
        void set_current_pitcher(Player new_pitcher);
        void set_position_in_field(Player new_player, eDefensivePositions position);

        void print_fielders();
        void print_batting_order();

        void reset();

    private:
        Player pick_starting_pitcher();
        Player pick_relief_pitcher();

        void set_up_batting_order();
        void set_up_fielders();
        void set_up_pitchers();

        std::set<Player> get_all_pitchers();

        std::set<Player> find_players(std::vector<std::string> player_ids) {
            std::set<Player> result;
            for (Player player : all_players) {
                for (std::string player_id : player_ids) {
                    if (player.id == player_id) {
                        result.insert(player);
                        break;
                    }
                }
            }
            return result;
        }

        Player find_best_player_for_defense_pos(eDefensivePositions position, std::vector<Player> players_to_exclude = {});
        eDefensivePositions find_player_in_fielders(Player player, eDefensivePositions = (eDefensivePositions)0, eDefensivePositions to = NUM_DEFENSIVE_POSITIONS);
};