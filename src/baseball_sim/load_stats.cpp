#include "load_stats.hpp"

#include "table.hpp"
#include "statistics.hpp"
#include "team.hpp"
#include "player.hpp"
#include "utils.hpp"

#include <unordered_map>

using namespace std;


const vector<ePlayer_Stat_Types> PLAYER_STATS_TO_ALWAYS_LOAD = {PLAYER_APPEARANCES};

League_Stats LEAGUE_AVG_STATS;
unordered_map<string, shared_ptr<Player>> player_cache;
unordered_map<string, shared_ptr<Team>> team_cache;

void Stat_Loader::load_league_avgs() {
    vector<map<string, string>> batting_data = read_csv_file(get_league_data_file_path("batting"));
    vector<map<string, string>> pitching_data = read_csv_file(get_league_data_file_path("pitching"));

    Stat_Table batting_table(batting_data, "batting", "league_avg_batting");
    Stat_Table pitching_table(pitching_data, "pitching", "league_avg_pitching");

    Stat_Table arr[NUM_LEAGUE_STAT_TYPES] = {batting_table, pitching_table};
    LEAGUE_AVG_STATS = League_Stats(arr);
}


Team* Stat_Loader::load_team(const string& team_abbreviation, int year) {
    const string cache_id = get_team_cache_id(team_abbreviation, year);
    if (is_team_cached(cache_id)) { // This probably causes issues if duplicate teams play each other but I'm not gonna worry about that
        return team_cache[cache_id].get();
    }

    Team_Stats team_stats = load_team_stats(team_abbreviation, year);
    vector<Player*> roster = load_team_roster(team_stats, year);
    Team team(team_abbreviation, roster, team_stats);

    return cache_team(team, cache_id);
}


Team_Stats Stat_Loader::load_team_stats(const string& team_abbreviation, int year) {
    Stat_Table team_stat_tables[NUM_TEAM_STAT_TYPES];

    for (int i = 0; i < NUM_TEAM_STAT_TYPES; i++) {
        string filename = get_team_data_file_path(team_abbreviation, year, TEAM_STAT_TYPES[i]);
        vector<map<string, string>> file_data = read_csv_file(filename);
        Stat_Table stat_table(file_data, TEAM_STAT_TYPES[i], team_abbreviation + "_" + to_string(year) + "_" + TEAM_STAT_TYPES[i]);
        team_stat_tables[i] = stat_table;
    }
    return Team_Stats(team_abbreviation, team_stat_tables, year);
}


vector<Player*> Stat_Loader::load_team_roster(Team_Stats& team_stats, int year) {
    vector<Player*> result;
    string team_abbreviation = team_stats.stat_tables[TEAM_INFO].get_stat<string>("abbreviation", 0, "NO ABBREVIATION FOUND");

    for (const Table_Row& player_data : team_stats.stat_tables[TEAM_ROSTER].get_rows()) {
        string player_id = player_data.get_stat<string>("ID", "");
        string name = player_data.get_stat<string>("name_display", "");
        vector<ePlayer_Stat_Types> stats_to_load(PLAYER_STATS_TO_ALWAYS_LOAD);

        for (eTeam_Stat_Types team_stat_type : {TEAM_BATTING, TEAM_PITCHING}) {
            vector<Table_Row> search_results = team_stats.stat_tables[team_stat_type].filter_rows({{"ID", {player_id}}});
            if (!search_results.empty()) {
                for (ePlayer_Stat_Types player_stat_type : TEAM_TO_PLAYER_STAT_CORRESPONDENCE[team_stat_type]) {
                    stats_to_load.push_back(player_stat_type);
                }
            }
        }

        result.push_back(load_player(name, player_id, year, team_abbreviation, stats_to_load));
    }

    return result;
}


Team* Stat_Loader::cache_team(const Team& team, const string& cache_id) {
    team_cache[cache_id] = make_shared<Team>(team);
    return team_cache[cache_id].get();
}


Player* Stat_Loader::load_player(const string& player_name, const string& player_id, int year, const string& team_abbreviation, const vector<ePlayer_Stat_Types>& stats_to_load) {
    const string cache_id = get_player_cache_id(player_id, team_abbreviation, year);
    if (is_player_cached(cache_id)) { // Check if player was loaded by a different team
        return player_cache.at(cache_id).get();
    }

    Player_Stats stats = load_necessary_player_stats(player_id, year, team_abbreviation, stats_to_load);
    Player new_player(player_name, stats);
    return cache_player(new_player, cache_id);
}


Player_Stats Stat_Loader::load_necessary_player_stats(const string& player_id, int year, const string& team_abbreviation, const vector<ePlayer_Stat_Types>& stats_to_load) {
    Stat_Table all_player_stats[NUM_PLAYER_STAT_TYPES];

    for (ePlayer_Stat_Types stat_type : stats_to_load) {
        Stat_Table stat_container = load_player_stat_table(player_id, team_abbreviation, year, stat_type);
        all_player_stats[stat_type] = stat_container;
    }
    return Player_Stats(player_id, year, team_abbreviation, all_player_stats);
}


Stat_Table Stat_Loader::load_player_stat_table(const string& player_id, const string& team_abbreviation, int year, ePlayer_Stat_Types player_stat_type) {
    string stat_type = PLAYER_STAT_TYPES[player_stat_type];
    string filename = get_player_data_file_path(player_id, stat_type);

    vector<map<string, string>> player_file_data = read_csv_file(filename);

    Stat_Table stat_container(player_file_data, stat_type, player_id + "_" + stat_type);
    return stat_container;
}


Player* Stat_Loader::cache_player(const Player& player, const string& cache_id) {
    player_cache[cache_id] = make_shared<Player>(player);
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


string Stat_Loader::get_team_data_file_path(const string& team_abbreviation, int year, const string& team_data_file_type) {
    return get_team_year_dir_path(team_abbreviation, year) + "/" + team_abbreviation + "_" + to_string(year) + "_" + team_data_file_type + ".csv";
}


string Stat_Loader::get_team_year_dir_path(const string& team_abbreviation, int year) {
    return TEAMS_FILE_PATH + "/" + team_abbreviation + "/" + to_string(year);
}


std::string Stat_Loader::get_league_data_file_path(const string& stat_type) {
    return LEAGUE_FILE_PATH  + "/" + "league_" + stat_type + "_avg.csv";
}