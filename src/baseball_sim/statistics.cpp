#include "statistics.hpp"

#include "table.hpp"

using namespace std;


string PLAYER_STAT_TYPES[NUM_PLAYER_STAT_TYPES] = {"batting", "pitching", "appearances", "baserunning"};
string TEAM_STAT_TYPES[NUM_TEAM_STAT_TYPES] = {"roster", "batting", "pitching", "common_batting_orders", "team_info"};

std::map<eTeam_Stat_Types, vector<ePlayer_Stat_Types>> TEAM_TO_PLAYER_STAT_CORRESPONDENCE = {{TEAM_BATTING, {PLAYER_BATTING, PLAYER_BASERUNNING}}, {TEAM_PITCHING, {PLAYER_PITCHING}}};

Player_Stats::Player_Stats(const string& player_id, int year_to_pull_stats_from, const string& team_abbreviation, Stat_Table<float> player_stat_tables[NUM_PLAYER_STAT_TYPES]) {
    this->player_id = player_id;
    this->current_year = year_to_pull_stats_from;
    this->current_team_abbreviation = team_abbreviation;

    for (int i = 0; i < NUM_PLAYER_STAT_TYPES; i++) {
        this->stat_tables[i] = player_stat_tables[i];
    }
}


Team_Stats::Team_Stats(const string& team_id, Stat_Table<string> team_stat_tables[NUM_TEAM_STAT_TYPES]) {
    this->team_name = team_id;
    for (int i = 0; i < NUM_TEAM_STAT_TYPES; i++) {
        this->stat_tables[i] = team_stat_tables[i];
    }
}


Table_Row<string> Team_Stats::get_row(eTeam_Stat_Types stat_type, const map<string, vector<string>>& row_attributes) {
    return stat_tables[stat_type].filter_rows(row_attributes)[0];
}


League_Stats::League_Stats(Stat_Table<float> league_stat_tables[NUM_LEAGUE_STAT_TYPES]) {
    for (int i = 0; i < NUM_LEAGUE_STAT_TYPES; i++) {
        stat_tables[i] = league_stat_tables[i];
    }
    start_year = stat_tables[0].get_stat("year_ID", 0, 0);
}