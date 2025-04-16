#include "statistics.hpp"

#include "table.hpp"

using namespace std;


string PLAYER_STAT_TYPES[NUM_PLAYER_STAT_TYPES] = {"batting", "pitching", "appearances", "baserunning", "batting_against"};
string TEAM_STAT_TYPES[NUM_TEAM_STAT_TYPES] = {"roster", "batting", "pitching", "common_batting_orders", "team_info", "schedule"};

map<eTeam_Stat_Types, vector<ePlayer_Stat_Types>> TEAM_TO_PLAYER_STAT_CORRESPONDENCE = {{TEAM_BATTING, {PLAYER_BATTING, PLAYER_BASERUNNING}}, {TEAM_PITCHING, {PLAYER_PITCHING, PLAYER_BATTING_AGAINST}}};

Player_Stats::Player_Stats(const string& player_id, int year_to_pull_stats_from, const string& team_abbreviation, Stat_Table player_stat_tables[NUM_PLAYER_STAT_TYPES]) {
    this->player_id = player_id;
    this->cache_id = get_player_cache_id(player_id, team_abbreviation, year_to_pull_stats_from);
    this->current_year = year_to_pull_stats_from;
    this->current_team_abbreviation = team_abbreviation;

    for (int i = 0; i < NUM_PLAYER_STAT_TYPES; i++) {
        this->stat_tables[i] = player_stat_tables[i];
        change_stat_table_target_row((ePlayer_Stat_Types)i, current_year, current_team_abbreviation);
    }
}


void Player_Stats::change_stat_table_target_row(ePlayer_Stat_Types stat_type, int year, const string& team_abbreviation) {
    string year_str = "year_id";
    string team_name_str = "team_name_abbr";

    if ((stat_type == PLAYER_BASERUNNING) || (stat_type == PLAYER_BATTING_AGAINST)) {
        year_str = "year_ID";
        team_name_str = "team_ID";
    }

    current_table_row_indices[stat_type] = stat_tables[stat_type].find_row(map<string, vector<Table_Entry>>({{year_str, {(float)year}}, {team_name_str, {team_abbreviation}}}));
    if (current_table_row_indices[stat_type] < 0) {
        current_table_row_indices[stat_type] = stat_tables[stat_type].get_rows().size() - 1;
    }
}



Team_Stats::Team_Stats(const string& main_team_abbreviation, Stat_Table team_stat_tables[NUM_TEAM_STAT_TYPES], int year) {
    for (int i = 0; i < NUM_TEAM_STAT_TYPES; i++) {
        this->stat_tables[i] = team_stat_tables[i];
    }
    this->main_team_abbreviation = main_team_abbreviation;
    this->year_specific_abbreviation = stat_tables[TEAM_INFO].get_stat<string>("abbreviation", 0, "NO ABBREVIATION FOUND");
    this->team_cache_id = get_team_cache_id(year_specific_abbreviation, year);
}


Table_Row Team_Stats::get_row(eTeam_Stat_Types stat_type, const map<string, vector<Table_Entry>>& row_attributes) {
    return stat_tables[stat_type].filter_rows(row_attributes)[0];
}


League_Stats::League_Stats(Stat_Table league_stat_tables[NUM_LEAGUE_STAT_TYPES]) {
    for (int i = 0; i < NUM_LEAGUE_STAT_TYPES; i++) {
        stat_tables[i] = league_stat_tables[i];
    }
    start_year = stat_tables[0].get_stat("year_ID", 0, .0f);
}