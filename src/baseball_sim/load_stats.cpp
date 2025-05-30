#include "load_stats.hpp"

#include "table.hpp"
#include "statistics.hpp"
#include "team.hpp"
#include "player.hpp"
#include "utils.hpp"

#include <unordered_map>
#include <algorithm>

using namespace std;


All_League_Stats_Wrapper ALL_LEAGUE_STATS;
unordered_map<string, unique_ptr<Player>> player_cache;
unordered_map<string, unique_ptr<Team>> team_cache;


Season Stat_Loader::load_season(uint year) {
    if (year <= 1948) {
        cerr << "ERROR: Simulating seasons earlier than (and including) 1948 is not supported. Please try a different season :)\n";
        throw exception();
    }

    load_league_year_stats(year);
    vector<string> real_team_abbrs = load_all_real_team_abbrs_from_year(year);
    vector<Team*> locally_saved_teams = load_all_saved_teams_from_year(year);
    
    for (const string& abbr : real_team_abbrs) {
        if (!is_team_cached(get_team_cache_id(abbr, year))) {
            cerr << "Data not found locally for "+ abbr + "_"+ to_string(year) + ". You must scrape stats for this team before simulating this season.\n";
            throw exception();
        }
    }
    return Season(locally_saved_teams, year);
}


// Returns the abbreviations of all the teams that played during this year.
// Note that it returns the abbreviations of each team's name in that year,
// not the main team abbreviations, so we cannot load directly from this list of abbreviations.
// NOTE: League stats for this year must be loaded before this is called.
std::vector<std::string> Stat_Loader::load_all_real_team_abbrs_from_year(uint year) {
    const Stat_Table& standings_table = ALL_LEAGUE_STATS[year][LEAGUE_STANDINGS];
    return standings_table.column<string>("ID", "NO ID FOUND");
}


vector<Team*> Stat_Loader::load_all_saved_teams_from_year(uint year) {
    const Stat_Table all_teams_table = load_all_teams_table();
    vector<string> team_abbreviations = all_teams_table.column<string>("TEAM_ID", "NO ID FOUND");

    vector<Team*> loaded_teams;
    for (const string& team_abbr : team_abbreviations) {
        string team_year_dir = get_team_year_dir_path(team_abbr, year);

        if (file_exists(team_year_dir)) { // If file doesn't exist, for now we just assume the team didn't exist that year (this is verified later)
            loaded_teams.push_back(load_team(team_abbr, year));
        }
    }
    return loaded_teams;
}


Stat_Table Stat_Loader::load_all_teams_table() {
    string filename = RESOURCES_FILE_PATH + "/all_teams.csv";
    map<string, vector<Table_Entry>> table_data = read_csv_file(filename);
    return Stat_Table(table_data, "all_teams");
}


void Stat_Loader::load_league_year_stats(uint year) {
    if (ALL_LEAGUE_STATS.holds_year(year)) return; // If this year is already loaded we don't need to do it again

    Stat_Table league_year_stat_tables[NUM_LEAGUE_STAT_TYPES];

    for (int i = 0; i < NUM_LEAGUE_STAT_TYPES; i++) {
        if (year < LEAGUE_STAT_EARLIEST_YEARS.at((eLeague_Stat_Types)i)) {
            continue;
        }
        const string filename = get_league_data_file_path(LEAGUE_STAT_NAMES[i], year);
        map<string, vector<Table_Entry>> file_data = read_csv_file(filename);
        league_year_stat_tables[i] = Stat_Table(file_data, filename);
    }

    ALL_LEAGUE_STATS.add_year(year, League_Stats(year, league_year_stat_tables));
}


Team* Stat_Loader::load_team(const string& main_team_abbreviation, uint year) {
    Team_Stats team_stats = load_team_stats(main_team_abbreviation, year);
    vector<Player*> roster = load_team_roster(team_stats, year);
    Team team(team_stats.year_specific_abbreviation, roster, team_stats);

    return cache_team(team, team_stats.team_cache_id);
}


Team_Stats Stat_Loader::load_team_stats(const string& main_team_abbreviation, uint year) {
    Stat_Table team_stat_tables[NUM_TEAM_STAT_TYPES];

    for (int i = 0; i < NUM_TEAM_STAT_TYPES; i++) {
        string filename = get_team_data_file_path(main_team_abbreviation, year, TEAM_STAT_NAMES[i]);
        map<string, vector<Table_Entry>> file_data = read_csv_file(filename);
        team_stat_tables[i] = Stat_Table(file_data, filename);
    }
    return Team_Stats(main_team_abbreviation, team_stat_tables, year);
}


vector<Player*> Stat_Loader::load_team_roster(Team_Stats& team_stats, uint year) {
    vector<Player*> result;
    for (size_t i = 0; i < team_stats[TEAM_ROSTER].size(); i++) {
        string player_id = team_stats.get_stat<string>(TEAM_ROSTER, "ID", i, "");
        string name = team_stats.get_stat<string>(TEAM_ROSTER, "name_display", i, "");

        vector<ePlayer_Stat_Types> stats_to_load = get_player_stat_types_to_load(player_id, team_stats);
        result.push_back(load_player(name, player_id, year, team_stats.year_specific_abbreviation, stats_to_load));
    }
    return result;
}


vector<ePlayer_Stat_Types> Stat_Loader::get_player_stat_types_to_load(const string& player_id, Team_Stats& team_stats) {
    vector<ePlayer_Stat_Types> stats_to_load;
    for (eTeam_Stat_Types team_stat_type : {TEAM_ROSTER, TEAM_BATTING, TEAM_PITCHING}) {
        vector<size_t> search_results = team_stats[team_stat_type].filter_rows({{"ID", {player_id}}});
        if (!search_results.empty()) {
            for (ePlayer_Stat_Types player_stat_type : TEAM_TO_PLAYER_STAT_CORRESPONDENCE[team_stat_type]) {
                if (should_load_player_stat_type(team_stats, search_results[0], player_stat_type)) {
                    stats_to_load.push_back(player_stat_type);
                }
            }
        }
    }
    return stats_to_load;
}


// There are cases where players may appear on a team stat list, but still should not have some of those stats loaded.
// Ex: Player appears on team batting list but has no hits, so we shouldn't load their baserunning stats (they never got on base).
bool Stat_Loader::should_load_player_stat_type(const Team_Stats& team_stats, size_t player_row, ePlayer_Stat_Types stat_type) {
    if (team_stats.year < PLAYER_STAT_EARLIEST_YEARS.at(stat_type)) {
        return false;
    }
    if ((stat_type == PLAYER_BASERUNNING) && (team_stats[TEAM_BATTING].get_stat("b_h", player_row, .0f) == 0)) {
        return false;
    }
    if ((stat_type == PLAYER_FIELDING) && (team_stats[TEAM_ROSTER].get_stat("games_defense", player_row, .0f) == 0)) {
        return false;
    }
    return true;
}


Team* Stat_Loader::cache_team(const Team& team, const string& cache_id) {
    team_cache[cache_id] = make_unique<Team>(team);
    return team_cache[cache_id].get();
}


Player* Stat_Loader::load_player(const string& player_name, const string& player_id, uint year, const string& team_abbreviation, const vector<ePlayer_Stat_Types>& stats_to_load) {
    const string cache_id = get_player_cache_id(player_id, team_abbreviation, year);
    if (is_player_cached(cache_id)) { // Check if player was loaded by a different team
        return player_cache.at(cache_id).get();
    }

    Player_Stats stats = load_necessary_player_stats(player_id, year, team_abbreviation, stats_to_load);
    Player new_player(player_name, stats);
    return cache_player(new_player, cache_id);
}


Player_Stats Stat_Loader::load_necessary_player_stats(const string& player_id, uint year, const string& team_abbreviation, const vector<ePlayer_Stat_Types>& stats_to_load) {
    Stat_Table all_player_stats[NUM_PLAYER_STAT_TYPES];

    for (ePlayer_Stat_Types stat_type : stats_to_load) {
        Stat_Table stat_table = load_player_stat_table(player_id, team_abbreviation, year, stat_type);
        all_player_stats[stat_type] = stat_table;
    }
    return Player_Stats(player_id, year, team_abbreviation, all_player_stats);
}


Stat_Table Stat_Loader::load_player_stat_table(const string& player_id, const string& team_abbreviation, uint year, ePlayer_Stat_Types player_stat_type) {
    string stat_type = PLAYER_STAT_NAMES[player_stat_type];
    string filename = get_player_data_file_path(player_id, stat_type);
    map<string, vector<Table_Entry>> player_file_data = read_csv_file(filename);

    Stat_Table stat_table(player_file_data, filename);
    return stat_table;
}


Player* Stat_Loader::cache_player(const Player& player, const string& cache_id) {
    player_cache[cache_id] = make_unique<Player>(player);
    return player_cache.at(cache_id).get();
}


bool Stat_Loader::is_team_cached(const string& team_cache_id) {
    return team_cache.count(team_cache_id) > 0;
}


bool Stat_Loader::is_player_cached(const string& player_cache_id) {
    return player_cache.count(player_cache_id) > 0;
}


string Stat_Loader::get_player_data_file_path(const string& player_id, const string& stat_type) {
    return PLAYERS_FILE_PATH + "/" + stat_type + "/" + player_id + "_" + stat_type + ".csv";
}


string Stat_Loader::get_team_data_file_path(const string& main_team_abbreviation, uint year, const string& team_data_file_type) {
    return get_team_year_dir_path(main_team_abbreviation, year) + "/" + main_team_abbreviation + "_" + to_string(year) + "_" + team_data_file_type + ".csv";
}


string Stat_Loader::get_team_year_dir_path(const string& main_team_abbreviation, uint year) {
    return TEAMS_FILE_PATH + "/" + main_team_abbreviation + "/" + to_string(year);
}


std::string Stat_Loader::get_league_data_file_path(const string& league_data_file_type, uint year) {
    return get_league_year_dir_path(year) + "/" + to_string(year) + "_league_" + league_data_file_type + ".csv";
}


std::string Stat_Loader::get_league_year_dir_path(uint year) {
    return LEAGUE_FILE_PATH + "/" + to_string(year);
}