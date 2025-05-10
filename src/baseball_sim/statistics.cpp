#include "statistics.hpp"

#include "config.hpp"
#include "table.hpp"
#include "utils.hpp"

using namespace std;


string PLAYER_STAT_NAMES[NUM_PLAYER_STAT_TYPES] = {"batting", "pitching", "appearances", "baserunning", "batting_against", "pitch_summary_batting", "pitch_summary_pitching"};
string TEAM_STAT_NAMES[NUM_TEAM_STAT_TYPES] = {"roster", "batting", "pitching", "common_batting_orders", "team_info", "schedule"};
string LEAGUE_STAT_NAMES[NUM_LEAGUE_STAT_TYPES] = {"batting", "pitching", "pitch_summary_batting", "pitch_summary_pitching", "standings"};

map<ePlayer_Stat_Types, unsigned int> PLAYER_STAT_EARLIEST_YEARS {
    {PLAYER_BATTING, 0},
    {PLAYER_PITCHING, 0},
    {PLAYER_APPEARANCES, 0},
    {PLAYER_BASERUNNING, 1912},
    {PLAYER_BATTING_AGAINST, 1912},
    {PLAYER_PITCH_SUMMARY_BATTING, 1988},
    {PLAYER_PITCH_SUMMARY_PITCHING, 1988}
};

map<eTeam_Stat_Types, vector<ePlayer_Stat_Types>> TEAM_TO_PLAYER_STAT_CORRESPONDENCE = {
    {TEAM_BATTING, {PLAYER_BATTING, PLAYER_BASERUNNING, PLAYER_PITCH_SUMMARY_BATTING}},
    {TEAM_PITCHING, {PLAYER_PITCHING, PLAYER_BATTING_AGAINST, PLAYER_PITCH_SUMMARY_PITCHING}}
};

map<eLeague_Stat_Types, unsigned int> LEAGUE_STAT_EARLIEST_YEARS {
    {LEAGUE_BATTING, 0},
    {LEAGUE_PITCHING, 0},
    {LEAGUE_PITCH_SUMMARY_BATTING, 1988},
    {LEAGUE_PITCH_SUMMARY_PITCHING, 1988},
    {LEAGUE_STANDINGS, 0}
};


Player_Stats::Player_Stats(const string& player_id, unsigned int year_to_pull_stats_from, const string& team_abbreviation, Stat_Table player_stat_tables[NUM_PLAYER_STAT_TYPES]) : Stat_Table_Container(player_stat_tables){
    this->player_id = player_id;
    this->cache_id = get_player_cache_id(player_id, team_abbreviation, year_to_pull_stats_from);
    this->current_year = year_to_pull_stats_from;
    this->current_team_abbreviation = team_abbreviation;

    for (int i = 0; i < NUM_PLAYER_STAT_TYPES; i++) {
        change_stat_table_target_row((ePlayer_Stat_Types)i, current_year, current_team_abbreviation);
    }
}


void Player_Stats::change_stat_table_target_row(ePlayer_Stat_Types stat_type, unsigned int year, const string& team_abbreviation) {
    string year_str = "year_id";
    string team_name_str = "team_name_abbr";

    if (is_player_stat_out_of_date(stat_type)) {
        year_str = "year_ID";
        team_name_str = "team_ID";
    }

    int target_row = stat_tables[stat_type].find_row(map<string, vector<Table_Entry>>({{year_str, {(float)year}}, {team_name_str, {team_abbreviation}}}));
    if (target_row < 0) {
        current_table_row_indices[stat_type] = stat_tables[stat_type].size() - 1;
        debug_line(
            if (!stat_tables[stat_type].empty())
                cout << "Missing stat row of player stat type "<< PLAYER_STAT_NAMES[stat_type] << " in non-empty stat table "<< stat_tables[stat_type].stat_table_id << " for player "<< cache_id << "\n";
        )
    }
    else {
        current_table_row_indices[stat_type] = target_row;
    }
}


// These tables have non-standard headers on baseball reference (at least until BR updates them)
bool is_player_stat_out_of_date(ePlayer_Stat_Types stat_type) {
    return (stat_type == PLAYER_BASERUNNING) || (stat_type == PLAYER_BATTING_AGAINST) || (stat_type == PLAYER_PITCH_SUMMARY_BATTING) || (stat_type == PLAYER_PITCH_SUMMARY_PITCHING);
}


Team_Stats::Team_Stats(const string& main_team_abbreviation, Stat_Table team_stat_tables[NUM_TEAM_STAT_TYPES], unsigned int year): Stat_Table_Container(team_stat_tables) {
    this->main_team_abbreviation = main_team_abbreviation;
    this->year_specific_abbreviation = stat_tables[TEAM_INFO].get_stat<string>("abbreviation", 0, "NO ABBREVIATION FOUND");
    this->team_cache_id = get_team_cache_id(year_specific_abbreviation, year);

    unsigned int start_date = get_day_of_year(get_stat<string>(TEAM_SCHEDULE, "date_game", 0, ""), year);
    unsigned int end_date = get_day_of_year(get_stat<string>(TEAM_SCHEDULE, "date_game", stat_tables[TEAM_SCHEDULE].size() - 1, ""), year);
    this->days_in_schedule = end_date - start_date;
    this->year = year;
}


bool All_League_Stats_Wrapper::holds_year(unsigned int year) const {
    return league_stat_tables.find(year) != league_stat_tables.end();
}


void All_League_Stats_Wrapper::add_year(unsigned int year, const League_Stats& year_table) {
    league_stat_tables.insert({year, year_table});
}


const League_Stats& All_League_Stats_Wrapper::get_year(unsigned int year) {
    return league_stat_tables.at(year);
}


unsigned int All_League_Stats_Wrapper::get_avg_pitcher_cooldown(unsigned int year) {
    const League_Stats& year_table = get_year(year);
    return year_table.get_stat(LEAGUE_PITCHING, "G", 0, 0.f)/year_table.get_stat(LEAGUE_PITCHING, "pitchers_used", 0, 1.f);
}