#include "statistics.hpp"

#include "includes.hpp"
#include "table.hpp"
#include "utils.hpp"

#include <assert.h>

using namespace std;

Global_Stat_Container global_stats;

string PLAYER_STAT_NAMES[NUM_PLAYER_STAT_TYPES] = {"batting", "pitching", "fielding", "appearances", "baserunning", "baserunning_against", "batting_against", "pitch_summary_batting", "pitch_summary_pitching"};
string TEAM_STAT_NAMES[NUM_TEAM_STAT_TYPES] = {"roster", "batting", "pitching", "common_batting_orders", "team_info", "schedule"};
string LEAGUE_STAT_NAMES[NUM_LEAGUE_STAT_TYPES] = {"batting", "pitching", "fielding", "baserunning", "pitch_summary_batting", "pitch_summary_pitching", "batting_by_bases", "standings"};

map<ePlayer_Stat_Types, unsigned int> PLAYER_STAT_EARLIEST_YEARS {
    {PLAYER_BATTING, 0},
    {PLAYER_PITCHING, 0},
    {PLAYER_FIELDING, 0},
    {PLAYER_APPEARANCES, 0},
    {PLAYER_BASERUNNING, 1912},
    {PLAYER_BASERUNNING_AGAINST, 1912},
    {PLAYER_BATTING_AGAINST, 1912},
    {PLAYER_PITCH_SUMMARY_BATTING, 1988},
    {PLAYER_PITCH_SUMMARY_PITCHING, 1988}
};

map<eTeam_Stat_Types, vector<ePlayer_Stat_Types>> TEAM_TO_PLAYER_STAT_CORRESPONDENCE = {
    {TEAM_ROSTER, {PLAYER_APPEARANCES, PLAYER_FIELDING}},
    {TEAM_BATTING, {PLAYER_BATTING, PLAYER_BASERUNNING, PLAYER_PITCH_SUMMARY_BATTING}},
    {TEAM_PITCHING, {PLAYER_PITCHING, PLAYER_BASERUNNING_AGAINST, PLAYER_BATTING_AGAINST, PLAYER_PITCH_SUMMARY_PITCHING}}
};

map<eLeague_Stat_Types, unsigned int> LEAGUE_STAT_EARLIEST_YEARS {
    {LEAGUE_BATTING, 0},
    {LEAGUE_PITCHING, 0},
    {LEAGUE_FIELDING, 0},
    {LEAGUE_BASERUNNING, 1912},
    {LEAGUE_PITCH_SUMMARY_BATTING, 1988},
    {LEAGUE_PITCH_SUMMARY_PITCHING, 1988},
    {LEAGUE_BATTING_BY_BASES, 1912},
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

// Change this for fielding
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
    return (stat_type == PLAYER_BASERUNNING)
        || (stat_type == PLAYER_BASERUNNING_AGAINST)
        || (stat_type == PLAYER_BATTING_AGAINST)
        || (stat_type == PLAYER_PITCH_SUMMARY_BATTING)
        || (stat_type == PLAYER_PITCH_SUMMARY_PITCHING);
}


Team_Stats::Team_Stats(const string& main_team_abbreviation, Stat_Table team_stat_tables[NUM_TEAM_STAT_TYPES], unsigned int year): Stat_Table_Container(team_stat_tables) {
    this->main_team_abbreviation = main_team_abbreviation;
    this->year_specific_abbreviation = stat_tables[TEAM_INFO].get_stat<string>("abbreviation", 0, "NO ABBREVIATION FOUND");
    this->team_cache_id = get_team_cache_id(year_specific_abbreviation, year);
    this->year = year;
    set_days_in_schedule();
}


// This aids in simulating seasons that aren't finished yet
void Team_Stats::set_days_in_schedule() {
    assert(stat_tables[TEAM_SCHEDULE].size() < 1000);
    assert(stat_tables[TEAM_SCHEDULE].size() > 0);

    unsigned int start_date = get_day_of_year(get_stat<string>(TEAM_SCHEDULE, "date_game", 0, ""), year);
    unsigned int end_date = start_date + 1;
    for (size_t i = stat_tables[TEAM_SCHEDULE].size(); i > 0; i--) {
        if (get_stat<string>(TEAM_SCHEDULE, "win_loss_result", i-1, "NONE") != "NONE") {
            end_date = get_day_of_year(get_stat<string>(TEAM_SCHEDULE, "date_game", i-1, ""), year);
            break;
        }
    }
    this->days_in_schedule = end_date - start_date;
}


League_Stats::League_Stats(unsigned int year, Stat_Table league_stat_tables[NUM_LEAGUE_STAT_TYPES]): Stat_Table_Container(league_stat_tables) {
    this->year = year;
    populate_probs();
}


void League_Stats::populate_probs() {
    populate_at_bat_probs();
    populate_steal_probs();
}


void League_Stats::populate_at_bat_probs() {
    if (year >= LEAGUE_STAT_EARLIEST_YEARS[LEAGUE_PITCH_SUMMARY_BATTING]) {
        strike_or_ball_probs[PITCH_STRIKE] = get_stat(LEAGUE_PITCH_SUMMARY_BATTING, "strike_perc", 0, .0f)/100;
        strike_or_ball_probs[PITCH_BALL] = 1 - strike_or_ball_probs[PITCH_STRIKE];

        strike_type_probs[STRIKE_LOOKING] = get_stat(LEAGUE_PITCH_SUMMARY_BATTING, "strike_looking_perc", 0, .0f)/100;
        strike_type_probs[STRIKE_SWINGING] = get_stat(LEAGUE_PITCH_SUMMARY_BATTING, "strike_swinging_perc", 0, .0f)/100;
        strike_type_probs[STRIKE_FOUL] = get_stat(LEAGUE_PITCH_SUMMARY_BATTING, "strike_foul_perc", 0, .0f)/100;
        strike_type_probs[STRIKE_IN_PLAY] = 1 - strike_type_probs[STRIKE_LOOKING] - strike_type_probs[STRIKE_SWINGING] - strike_type_probs[STRIKE_FOUL];
    }

    int league_plate_appearances = get_stat(LEAGUE_BATTING, "PA", 0, 1.f);
    basic_at_bat_probs[OUTCOME_STRIKEOUT] = get_stat(LEAGUE_BATTING, "SO", 0, .0f)/league_plate_appearances;
    basic_at_bat_probs[OUTCOME_WALK] = (get_stat(LEAGUE_BATTING, "BB", 0, .0f) + get_stat(LEAGUE_BATTING, "HBP", 0, .0f))/league_plate_appearances;
    basic_at_bat_probs[OUTCOME_BALL_IN_PLAY] = 1 - basic_at_bat_probs[OUTCOME_STRIKEOUT] - basic_at_bat_probs[OUTCOME_WALK];

    float league_balls_in_play = get_stat(LEAGUE_BATTING, "PA", 0, .0f) - get_stat(LEAGUE_BATTING, "BB", 0, .0f)
                               - get_stat(LEAGUE_BATTING, "HBP", 0, .0f) - get_stat(LEAGUE_BATTING, "SO", 0, .0f);
    float league_hits = get_stat(LEAGUE_BATTING, "H", 0, .0f);
    hit_or_out_probs[0] = league_hits/league_balls_in_play;
    hit_or_out_probs[1] = 1 - hit_or_out_probs[0];

    const int league_total_hits = get_stat(LEAGUE_BATTING, "H", 0, 1.f);
    hit_type_probs[1] = get_stat(LEAGUE_BATTING, "2B", 0, .0f)/league_total_hits;
    hit_type_probs[2] = get_stat(LEAGUE_BATTING, "3B", 0, .0f)/league_total_hits;
    hit_type_probs[3] = get_stat(LEAGUE_BATTING, "HR", 0, .0f)/league_total_hits;
    hit_type_probs[0] = 1 - hit_type_probs[1] - hit_type_probs[2] - hit_type_probs[3];
}


void League_Stats::populate_steal_probs() {
    if (year < LEAGUE_STAT_EARLIEST_YEARS[LEAGUE_BASERUNNING]) return;

    float total_sbo = get_stat(LEAGUE_BASERUNNING, "SB_opp", 0, 1.f);
    float sbos_on_first = get_stat(LEAGUE_BATTING_BY_BASES, "PA", 3, .0f) + get_stat(LEAGUE_BATTING_BY_BASES, "PA", 7, .0f); // These row values are hardcoded unfortunately
    sbo_on_first_percent = sbos_on_first/total_sbo;

    steal_attempt_probs[FIRST_BASE][1] = (get_stat(LEAGUE_BASERUNNING, "SB_2", 0, .0f) + get_stat(LEAGUE_BASERUNNING, "CS_2", 0, .0f))/sbos_on_first;
    steal_attempt_probs[FIRST_BASE][0] = 1 - steal_attempt_probs[FIRST_BASE][1];

    steal_attempt_probs[SECOND_BASE][1] = (get_stat(LEAGUE_BASERUNNING, "SB_3", 0, .0f) + get_stat(LEAGUE_BASERUNNING, "CS_3", 0, .0f))/(total_sbo - sbos_on_first);
    steal_attempt_probs[SECOND_BASE][0] = 1 - steal_attempt_probs[SECOND_BASE][1];

    for (int base = 0; base <= SECOND_BASE; base++) {
        float steals = get_stat(LEAGUE_BASERUNNING, BASE_STEALING_STAT_STRINGS[base][0], 0, .0f);
        float caught = get_stat(LEAGUE_BASERUNNING, BASE_STEALING_STAT_STRINGS[base][1], 0, .0f);
        float fielding_perc = get_stat(LEAGUE_FIELDING, "fielding_perc", 0, .0f);
        steal_success_probs[base][1] = fielding_perc*steals/(steals + caught);
        steal_success_probs[base][0] = 1 - steal_success_probs[base][1];
    }
}


bool All_League_Stats_Wrapper::holds_year(unsigned int year) const {
    return league_stat_tables.find(year) != league_stat_tables.end();
}


void All_League_Stats_Wrapper::add_year(unsigned int year, const League_Stats& year_table) {
    league_stat_tables.insert({year, year_table});
}


const League_Stats& All_League_Stats_Wrapper::get_year(unsigned int year) const {
    return league_stat_tables.at(year);
}