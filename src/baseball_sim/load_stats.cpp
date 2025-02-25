#include "load_stats.hpp"

#include "table.hpp"
#include "statistics.hpp"
#include "team.hpp"
#include "player.hpp"

#include <unordered_map>

using namespace std;


const vector<ePlayer_Stat_Types> PLAYER_STATS_TO_ALWAYS_LOAD = {PLAYER_APPEARANCES};

League_Stats LEAGUE_AVG_STATS;
std::unordered_map<string, shared_ptr<Player>> player_cache;

void Stat_Loader::load_league_avgs() {
    vector<map<string, string>> batting_data = read_csv_file(get_league_data_file_path("batting"));
    vector<map<string, string>> pitching_data = read_csv_file(get_league_data_file_path("pitching"));

    Stat_Table batting_table(batting_data, "batting", "league_avg_batting");
    Stat_Table pitching_table(pitching_data, "pitching", "league_avg_pitching");

    Stat_Table arr[NUM_LEAGUE_STAT_TYPES] = {batting_table, pitching_table};
    LEAGUE_AVG_STATS = League_Stats(arr);
}


Team Stat_Loader::load_team(const string& team_abbreviation, int year) {
    Team_Stats team_stats = load_team_stats(team_abbreviation, year);
    vector<Player*> roster = load_team_roster(team_stats, year);
    Team team(team_abbreviation, roster, team_stats);

    return team;
}


vector<Player*> Stat_Loader::load_team_roster(Team_Stats& team_stats, int year) {
    vector<Player*> result;
    string team_abbreviation = team_stats.stat_tables[TEAM_INFO].get_stat<string>("abbreviation", 0, "NO ABBREVIATION FOUND");

    for (const Table_Row& player_data : team_stats.stat_tables[TEAM_ROSTER].get_rows()) {
        string player_id = player_data.get_stat("ID", "");
        string name = player_data.get_stat("name_display", "");
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


Player* Stat_Loader::load_player(const string& player_name, const string& player_id, int year, const string& team_abbreviation, const vector<ePlayer_Stat_Types>& stats_to_load) {
    Player_Stats stats = load_necessary_player_stats(player_id, year, team_abbreviation, stats_to_load);
    Player new_player(player_name, stats);
    return cache_player(new_player);
}


Player_Stats Stat_Loader::load_necessary_player_stats(const string& player_id, int year, const string& team_abbreviation, const vector<ePlayer_Stat_Types>& stats_to_load) {
    Stat_Table all_player_stats[NUM_PLAYER_STAT_TYPES];

    for (ePlayer_Stat_Types stat_type : stats_to_load) {
        Stat_Table stat_container = load_player_stat_table(player_id, stat_type);
        all_player_stats[stat_type] = stat_container;
    }
    return Player_Stats(player_id, year, team_abbreviation, all_player_stats);
}


Stat_Table Stat_Loader::load_player_stat_table(const string& player_id, ePlayer_Stat_Types player_stat_type) {
    string stat_type = PLAYER_STAT_TYPES[player_stat_type];
    string filename = get_player_data_file_path(player_id, stat_type);
    vector<map<string, string>> player_file_data = read_csv_file(filename);

    Stat_Table stat_container(player_file_data, stat_type, player_id + "_" + stat_type);
    return stat_container;
}


Player* Stat_Loader::cache_player(const Player& player) {
    player_cache[player.id] = make_shared<Player>(player);
    return player_cache.at(player.id).get();
}


Team_Stats Stat_Loader::load_team_stats(const string& team_abbreviation, int year) {
    Stat_Table team_stat_tables[NUM_TEAM_STAT_TYPES];

    for (int i = 0; i < NUM_TEAM_STAT_TYPES; i++) {
        string filename = get_team_data_file_path(team_abbreviation, year, TEAM_STAT_TYPES[i]);
        vector<map<string, string>> file_data = read_csv_file(filename);
        Stat_Table stat_table(file_data, TEAM_STAT_TYPES[i], team_abbreviation + "_" + to_string(year) + "_" + TEAM_STAT_TYPES[i]);
        team_stat_tables[i] = stat_table;
    }
    return Team_Stats(team_abbreviation, team_stat_tables);
}



vector<map<string, string>> Stat_Loader::read_csv_file(const string& filename) {
    vector<map<string, string>> result;
    ifstream csv_file = open_file(filename);

    string current_line;
    getline(csv_file, current_line);

    vector<string> headers = read_csv_line(current_line);

    while (getline(csv_file, current_line)) {
        vector<string> line_data = read_csv_line(current_line);
        map<string, string> parsed_line_data = match_keys_to_values(headers, line_data);
        result.push_back(parsed_line_data);
    }

    return result;
}


ifstream Stat_Loader::open_file(const string& filename) {
    ifstream file(filename);
    if (!file.is_open()) {
        throw runtime_error("Could not open file " + filename);
    }
    else if (!file.good()) {
        throw runtime_error("There was an issue with file " + filename);
    }
    else {
        return file;
    }
}


/* Reads a line of a csv file and returns a list of all the comma-seperated values in that line. */
vector<string> Stat_Loader::read_csv_line(const string& line) {
    vector<string> result;
    stringstream stringstream(line);
    string current_item;

    while (getline(stringstream, current_item, ',')) {
        result.push_back(current_item);
    }

    return result;
}


/*
Creates a dictionary from a list of keys and values.
If there are fewer values than keys, the extra keys will be inserted with an empty string as their value.
If there are more values than keys, the extra values will be ignored.
*/
map<string, string> Stat_Loader::match_keys_to_values(const vector<string>& keys, const vector<string>& values) {
    map<string, string> result;

    for (unsigned int i = 0; i < keys.size(); i++) {
        if (i < values.size()) {
            result.insert( {keys[i], values[i]} );
        }
        else {
            result.insert( {keys[i], ""} );
        }
    }
    return result;
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