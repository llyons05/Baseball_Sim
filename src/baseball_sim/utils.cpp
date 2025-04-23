#include "utils.hpp"

#include <fstream>
#include <sstream>
#include <vector>
#include <map>
#include <iostream>
#include <variant>
#include <filesystem>

using namespace std;


bool file_exists(const string& filename) {
    return filesystem::exists(filename);
}


ifstream open_file(const string& filename) {
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


vector<map<string, string>> read_csv_file(const string& filename) {
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


/* Reads a line of a csv file and returns a list of all the comma-seperated values in that line. */
vector<string> read_csv_line(const string& line) {
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
map<string, string> match_keys_to_values(const vector<string>& keys, const vector<string>& values) {
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


vector<map<string, variant<monostate, float, string>>> convert_rows_to_table_entries(const vector<map<string, string>>& rows) {
    vector<map<string, variant<monostate, float, string>>> result;
    for (const map<string, string>& row : rows) {
        result.push_back(convert_row_to_table_entry(row));
    }
    return result;
}


map<string, variant<monostate, float, string>> convert_row_to_table_entry(const map<string, string>& row) {
    map<string, variant<monostate, float, string>> result;
    for (auto const& [key, value] : row) {
        variant<monostate, float, string> converted_val;
        if (value.empty()) {
            converted_val = monostate{};
        }
        else if (is_float(value)) {
            converted_val = stof(value);
        }
        else {
            converted_val = value;
        }
        result.insert({key, converted_val});
    }
    return result;
}


// please dont call this with an empty string
bool is_float(const string& str) {
    char* ptr;
    strtof(str.c_str(), &ptr);
    return (*ptr) == '\0';
}


// We must cache players with their team and year, since it is possible for the same player to play for two teams in the same year
// Also we might want to have two different versions of the same team play (ex: 2023 NYY vs 2024 NYY)
string get_player_cache_id(const string& player_id, const string& team_abbreviation, int year) {
    return player_id + "_" + team_abbreviation + "_" + to_string(year);
}


string get_player_cache_id(const string& player_id, const string& team_cache_id) {
    return player_id + "_" + team_cache_id;
}


string get_team_cache_id(const string& team_abbreviation, int year) {
    return team_abbreviation + "_" + to_string(year);
}