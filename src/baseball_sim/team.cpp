#include "team.hpp"

#include "statistics.hpp"
#include "player.hpp"
#include "table.hpp"

#include <vector>
#include <fstream>
#include <algorithm>
#include <random>
#include <set>

Team::Team(const std::string& team_name, const std::vector<Player*>& players, const Team_Stats& team_stats): batting_order(), fielders() {
    this->team_stats = team_stats;
    this->team_name = team_name;

    all_players = players;
    position_in_batting_order = 0;
    runs_allowed_by_pitcher = 0;

    set_up_pitchers();
    set_current_pitcher(pick_starting_pitcher());
    set_up_batting_order();
    set_up_fielders();
}


std::set<Player*> Team::filter_players_by_listed_pos(const std::vector<std::string>& positions) {
    std::vector<std::string> player_ids;

    for (eTeam_Stat_Types team_stat_type : {TEAM_BATTING, TEAM_PITCHING}) {
        Stat_Table<std::string>* stat_table = &team_stats.stat_tables[team_stat_type];

        std::vector<Table_Row<std::string>> search_results = stat_table->filter_rows({{"team_position", positions}});

        for (const Table_Row<std::string>& search_result : search_results) {
            player_ids.push_back(search_result.get_stat("ID", ""));
        }
    }

    return find_players(player_ids);
}


std::set<Player*> Team::filter_pitchers(const std::vector<std::string>& pitcher_types) {
    Stat_Table<std::string>* stat_table = &team_stats.stat_tables[TEAM_PITCHING];

    std::vector<Table_Row<std::string>> search_results = stat_table->filter_rows({{"team_position", pitcher_types}});
    std::vector<std::string> pitcher_ids;

    for (const Table_Row<std::string>& search_result : search_results) {
        pitcher_ids.push_back(search_result.get_stat("ID", ""));
    }

    return find_players(pitcher_ids);
}


std::set<Player*> Team::get_all_pitchers() {
    std::vector<Table_Row<std::string>> pitcher_table = team_stats.stat_tables[TEAM_PITCHING].get_rows();
    std::vector<std::string> pitcher_ids;

    for (const Table_Row<std::string>& row : pitcher_table) {
        pitcher_ids.push_back(row.get_stat("ID", ""));
    }

    return find_players(pitcher_ids);
}


Player* Team::pick_starting_pitcher() {
    Player* new_pitcher = get_pitcher();
    int max_games = -1;
    for (Player* player : available_pitchers) {
        int games = player->stats.get_stat(PLAYER_PITCHING, "p_gs", 0);
        if (games > max_games) {
            max_games = games;
            new_pitcher = player;
        }
    }

    return new_pitcher;
}


Player* Team::pick_relief_pitcher() {
    Player* new_pitcher = get_pitcher();
    float highest_win_loss = -1;
    for (Player* player : available_pitchers) {
        float win_loss = player->stats.get_stat(PLAYER_PITCHING, "p_sv", 0);
        if (win_loss > highest_win_loss) {
            highest_win_loss = win_loss;
            new_pitcher = player;
        }
    }

    return new_pitcher;
}


void Team::set_current_pitcher(Player* new_pitcher) {
    if (fielders[POS_DH] == fielders[POS_PITCHER]) {
        fielders[POS_DH] = new_pitcher;
        for (int i = 8; i >= 0; i--) { // Loop backwards since pitcher is almost always batting last
            if (batting_order[i] == fielders[POS_PITCHER]) {
                batting_order[i] = new_pitcher;
                break;
            }
        }
    }
    fielders[POS_PITCHER] = new_pitcher;
    available_pitchers.erase(new_pitcher);
    runs_allowed_by_pitcher = 0;
}


Player* Team::try_switching_pitcher(int current_half_inning) {
    float curr_pitcher_era = get_pitcher()->stats.get_stat(PLAYER_PITCHING, "p_earned_run_avg", 10.0);
    if (runs_allowed_by_pitcher > curr_pitcher_era) {
        Player* new_pitcher = pick_next_pitcher(current_half_inning);
        set_current_pitcher(new_pitcher);

        #if BASEBALL_DEBUG
        std::cout << "NEW PITCHER FOR " << team_name << ": " << new_pitcher->name << "\n";
        #endif

        return new_pitcher;
    }
    return get_pitcher();
}


Player* Team::pick_next_pitcher(int current_half_inning) {
    if (current_half_inning > 12) {
        return pick_relief_pitcher();
    }
    return pick_starting_pitcher();
}


// Pitcher must be set before calling
void Team::set_up_batting_order() {
    int max_games_found = -1;

    // Find the most used batting order
    Table_Row<std::string> most_common_batting_order;
    for (const Table_Row<std::string>& row : team_stats.stat_tables[TEAM_COMMON_BATTING_ORDERS].get_rows()) {
        int games = std::stoi(row.get_stat("games", "0"));
        if (games > max_games_found) {
            most_common_batting_order = row;
            max_games_found = games;
        }
    }
    // Loop through that batting order, add each player to our current batting order
    for (int i = 1; i < 10; i++) {
        std::string pos_in_order = std::to_string(i);
        std::string player_id = most_common_batting_order.get_stat(pos_in_order, "");
        if (player_id == "Pitcher") {
            batting_order[i - 1] = fielders[POS_PITCHER];
        }
        else {
            std::set<Player*> search_results = find_players({player_id});
            if (search_results.size()) {
                batting_order[i - 1] = *search_results.begin();
            }
        }
    }
}


// Pitcher and batting order must be set before calling this
void Team::set_up_fielders() {
    std::vector<std::string> players_added;
    for (int i = 1; i < POS_DH; i++) {
        eDefensivePositions pos_value = (eDefensivePositions)i;
        Player* best_player = find_best_player_for_defense_pos(pos_value, players_added);
        set_position_in_field(best_player, pos_value);
        players_added.push_back(best_player->id);
    }

    bool pitcher_is_batting = false;
    for (int i = 0; i < 9; i++) {
        if (fielders[POS_PITCHER] == batting_order[i]) {
            pitcher_is_batting = true;
            break;
        }
    }

    Player* dh;
    if (pitcher_is_batting)
        dh = fielders[POS_PITCHER];
    else
        dh = find_best_player_for_defense_pos(POS_DH, players_added);
    
    set_position_in_field(dh, POS_DH);
}


void Team::set_up_pitchers() {
    available_pitchers = get_all_pitchers();
}


void Team::set_position_in_field(Player* new_player, eDefensivePositions position) {
    fielders[position] = new_player;
}


// NOTE: only returns players currently in the batting order
Player* Team::find_best_player_for_defense_pos(eDefensivePositions position, const std::vector<std::string>& players_to_exclude) {
    int max_games_found = -1;
    Player* best_player = batting_order[0];
    
    for (int i = 0; i < 9; i++) {
        int games_at_pos = batting_order[i]->games_at_fielding_position(position);
        if ((games_at_pos > max_games_found) && (std::find(players_to_exclude.begin(), players_to_exclude.end(), batting_order[i]->id) == players_to_exclude.end())) {
            max_games_found = games_at_pos;
            best_player = batting_order[i];
        }
    }

    return best_player;
}


void Team::print_fielders() {
    for (int i = 0; i < NUM_DEFENSIVE_POSITIONS; i++) {
        std::cout << fielders[i]->name << " is starting at " << DEFENSIVE_POSITIONS[i] << "\n";
    }
    std::cout << "\n";
}


void Team::print_batting_order() {
    for (int i = 0; i < 9; i++) {
        std::cout << batting_order[i]->name << " is batting at " << std::to_string(i + 1) << "\n";
        std::cout << "\t-Batting Avg: " << batting_order[i]->stats.get_stat(PLAYER_BATTING, "b_batting_avg", 0) << "\n";
    }
    std::cout << "\n";
}


void Team::reset() {
    position_in_batting_order = 0;
    runs_allowed_by_pitcher = 0;
    set_up_pitchers();
    set_current_pitcher(pick_starting_pitcher());
}