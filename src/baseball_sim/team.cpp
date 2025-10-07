#include "team.hpp"

#include "includes.hpp"
#include "statistics.hpp"
#include "player.hpp"
#include "table.hpp"

#include <vector>
#include <fstream>
#include <algorithm>
#include <random>
#include <set>
#include <cassert>


using namespace std;


Team::Team(const string& team_name, const vector<Player*>& players, const Team_Stats& team_stats): 
    team_stats(team_stats),
    uses_dh(true),
    all_players(players), 
    batting_order(), 
    fielders() 
{
    this->team_name = team_name;

    position_in_batting_order = 0;
    runs_allowed_by_pitcher = 0;
    current_pitcher_starting_half_inning = 0;

    prepare_for_game(0, false);
}

// Call this before every game the team plays
void Team::prepare_for_game(uint day_of_game, bool keep_batting_order) {
    position_in_batting_order = 0;
    runs_allowed_by_pitcher = 0;
    current_pitcher_starting_half_inning = 0;

    set_up_pitchers();
    set_current_pitcher(pick_next_pitcher(0, day_of_game), 0);
    if (!keep_batting_order) {
        set_up_batting_order();
        set_up_fielders();
    }
}


set<Player*> Team::filter_players_by_listed_pos(const vector<Table_Entry>& positions) {
    vector<string> player_ids;

    for (eTeam_Stat_Types team_stat_type : {TEAM_BATTING, TEAM_PITCHING}) {
        const Stat_Table& stat_table = team_stats[team_stat_type];
        vector<size_t> search_results = stat_table.filter_rows({{"team_position", positions}});

        for (size_t search_result : search_results) {
            player_ids.push_back(stat_table.get_stat<string>("ID", search_result, ""));
        }
    }

    return find_players(player_ids);
}


set<Player*> Team::filter_pitchers(const vector<Table_Entry>& pitcher_types) {
    const Stat_Table& stat_table = team_stats[TEAM_PITCHING];
    vector<size_t> search_results = stat_table.filter_rows({{"team_position", pitcher_types}});
    vector<string> pitcher_ids;

    for (size_t search_result : search_results) {
        pitcher_ids.push_back(stat_table.get_stat<string>("ID", search_result, ""));
    }

    return find_players(pitcher_ids);
}


set<Player*> Team::get_all_pitchers() {
    vector<string> pitcher_ids = team_stats[TEAM_PITCHING].column<string>("ID", "");
    return find_players(pitcher_ids);
}


Player* Team::pick_starting_pitcher(uint current_day_of_year) {
    Player* new_pitcher = get_pitcher();
    Player* least_unrested_pitcher = get_pitcher();
    int max_games = -1;
    uint most_days_of_rest_for_unrested_player = 0;

    for (Player* player : available_pitchers) {
        int games_started = player->stats.get_stat(PLAYER_PITCHING, "p_gs", .0f);
        if (games_started <= 0) continue; // This player has never been a starting pitcher, so we don't want to put him in

        uint cooldown = min(team_stats.days_in_schedule/games_started, MAX_PITCHER_COOLDOWN);
        uint days_of_rest = current_day_of_year - player->day_of_last_game_played;
        bool is_rested = days_of_rest >= cooldown;

        if (is_rested && (games_started > max_games)) { // Also use winrate here
            max_games = games_started;
            new_pitcher = player;
        }
        else if ((max_games == -1) && (days_of_rest >= most_days_of_rest_for_unrested_player)) { // if our player is unrested and we are yet to find a rested player
            most_days_of_rest_for_unrested_player = days_of_rest;
            least_unrested_pitcher = player;
        }
    }
    if (max_games == -1) { // If there are no rested pitchers (this is somewhat rare), then we just go with the player that has the most rest
        new_pitcher = least_unrested_pitcher;
        game_viewer_line(debug_print("No rested starting pitchers available on " << team_stats.team_cache_id << ", defaulting to least unrested player..."));
    }

    return new_pitcher;
}


Player* Team::pick_relief_pitcher(uint current_day_of_year) {
    Player* new_pitcher = get_pitcher();
    Player* least_unrested_pitcher = get_pitcher();
    int most_relief_games = -1;
    uint most_days_of_rest_for_unrested_player = 0;

    for (Player* player : available_pitchers) {
        int games_total = player->stats.get_stat(PLAYER_PITCHING, "p_g", .0f);
        int relief_games = games_total - player->stats.get_stat(PLAYER_PITCHING, "p_gs", .0f);
        if (relief_games <= 0) continue; // If this player is only a starter, we do not put them in as a reliever. This helps save starting pitchers.

        uint cooldown = min(team_stats.days_in_schedule/games_total, MAX_PITCHER_COOLDOWN);
        uint days_of_rest = current_day_of_year - player->day_of_last_game_played;
        bool is_rested = days_of_rest >= cooldown;

        if (is_rested && (relief_games > most_relief_games)) {
            most_relief_games = relief_games;
            new_pitcher = player;
        }
        else if ((most_relief_games == -1) && (days_of_rest >= most_days_of_rest_for_unrested_player)) { // if our player is unrested and we are yet to find a rested player
            most_days_of_rest_for_unrested_player = days_of_rest;
            least_unrested_pitcher = player;
        }
    }
    if (most_relief_games == -1) { // If there are no rested pitchers (this is somewhat rare), then we just go with the player that has the most rest
        new_pitcher = least_unrested_pitcher;
        game_viewer_line(debug_print("No rested relief pitchers available on " << team_stats.team_cache_id << ", defaulting to least unrested player..."));
    }

    return new_pitcher;
}


void Team::set_current_pitcher(Player* new_pitcher, uint8_t current_half_inning) {
    if (!uses_dh) {
        debug_line(assert(fielders[POS_DH] == fielders[POS_PITCHER]));
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
    pitchers_used.push_back(new_pitcher);
    runs_allowed_by_pitcher = 0;
    current_pitcher_starting_half_inning = current_half_inning;
}


Player* Team::try_switching_pitcher(uint8_t current_half_inning, uint current_day_of_year) {
    if (should_swap_pitcher(get_pitcher(), current_half_inning)) {
        Player* new_pitcher = pick_next_pitcher(current_half_inning, current_day_of_year);
        set_current_pitcher(new_pitcher, current_half_inning);
        game_viewer_print("NEW PITCHER FOR " << team_name << ": " << new_pitcher->name << "\n");
        return new_pitcher;
    }
    return get_pitcher();
}


bool Team::should_swap_pitcher(Player* pitcher, uint8_t current_half_inning) {
    const float league_era = ALL_LEAGUE_STATS.get_stat(LEAGUE_PITCHING, team_stats.year, "earned_run_avg", .0f);
    if (runs_allowed_by_pitcher > league_era + 1) return true;

    const float total_innings = pitcher->stats.get_stat(PLAYER_PITCHING, "p_ip", .0f);
    float total_games = pitcher->stats.get_stat(PLAYER_PITCHING, "p_g", .0f);
    if (total_games == 0) total_games = 1;

    return ((current_half_inning - current_pitcher_starting_half_inning)/2) > (total_innings/total_games + 1);
}


Player* Team::pick_next_pitcher(uint8_t current_half_inning, uint current_day_of_year) {
    if (current_half_inning > 5) {
        return pick_relief_pitcher(current_day_of_year);
    }
    return pick_starting_pitcher(current_day_of_year);
}


// Pitcher must be set before calling
void Team::set_up_batting_order() {
    const Stat_Table& batting_order_table = team_stats[TEAM_COMMON_BATTING_ORDERS];
    uses_dh = true;

    // Finding the most used batting order (in the future use discrete distribution and select randomly)
    int max_games_found = -1;
    int most_common_batting_order_row = -1;
    for (size_t i = 0; i < batting_order_table.size(); i++) {
        int games = batting_order_table.get_stat("games", i, .0f);
        if (games > max_games_found) {
            most_common_batting_order_row = i;
            max_games_found = games;
        }
    }
    // Loop through that batting order, add each player to our current batting order
    for (int i = 1; i < 10; i++) {
        string pos_in_order = to_string(i);
        string player_id = batting_order_table.get_stat<string>(pos_in_order, most_common_batting_order_row, "");
        if (player_id == "Pitcher") {
            batting_order[i - 1] = fielders[POS_PITCHER];
            uses_dh = false;
        }
        else {
            set<Player*> search_results = find_players({player_id});
            if (search_results.size()) {
                batting_order[i - 1] = *search_results.begin();
            }
        }
    }
}


// Pitcher and batting order must be set before calling this
void Team::set_up_fielders() {
    vector<string> players_added;
    for (int i = POS_CATCHER; i < POS_DH; i++) {
        eDefensivePositions pos_value = (eDefensivePositions)i;
        Player* best_player = find_best_player_for_defense_pos(pos_value, players_added);
        set_position_in_field(best_player, pos_value);
        players_added.push_back(best_player->id);
    }

    Player* dh;
    if (uses_dh)
        dh = find_best_player_for_defense_pos(POS_DH, players_added);
    else
        dh = fielders[POS_PITCHER];
    
    set_position_in_field(dh, POS_DH);
}


void Team::set_up_pitchers() {
    available_pitchers = get_all_pitchers();
    pitchers_used.clear();
}


void Team::set_position_in_field(Player* new_player, eDefensivePositions position) {
    fielders[position] = new_player;
}


// NOTE: only returns players currently in the batting order
Player* Team::find_best_player_for_defense_pos(eDefensivePositions position, const vector<string>& players_to_exclude) {
    int max_games_found = -1;
    Player* best_player = batting_order[0];
    
    for (int i = 0; i < 9; i++) {
        int games_at_pos = batting_order[i]->games_at_fielding_position(position);
        if ((games_at_pos > max_games_found) && (find(players_to_exclude.begin(), players_to_exclude.end(), batting_order[i]->id) == players_to_exclude.end())) {
            max_games_found = games_at_pos;
            best_player = batting_order[i];
        }
    }

    return best_player;
}


void Team::reset_player_tracking_data() {
    for (Player* player : all_players) {
        player->day_of_last_game_played = 1000;
    }
}


void Team::print_fielders() {
    cout << team_name + " fielders:\n";
    for (int i = 0; i < NUM_DEFENSIVE_POSITIONS; i++) {
        cout << fielders[i]->name << " is starting at " << DEFENSIVE_POSITIONS[i] << "\n";
    }
    cout << "\n";
}


void Team::print_batting_order() {
    cout << team_name + " batting order:\n";
    for (int i = 0; i < 9; i++) {
        cout << to_string(i + 1) << ": " << batting_order[i]->name << "\n";
        cout << "\t-Batting Avg: " << batting_order[i]->stats.get_stat(PLAYER_BATTING, "b_batting_avg", .0f) << "\n";
    }
    cout << "\n";
}