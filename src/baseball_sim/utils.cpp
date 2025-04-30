#include "utils.hpp"

#include "table.hpp"

#include <fstream>
#include <sstream>
#include <vector>
#include <map>
#include <iostream>
#include <variant>
#include <filesystem>
#include <ctime>

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


map<string, vector<Table_Entry>> read_csv_file(const string& filename) {
    map<string, vector<Table_Entry>> result;
    ifstream csv_file = open_file(filename);

    string current_line;
    getline(csv_file, current_line);

    const vector<string> headers = read_csv_line(current_line);
    for (const string& header : headers) {
        result[header] = vector<Table_Entry>();
    }

    while (getline(csv_file, current_line)) {
        vector<string> line_data = read_csv_line(current_line);
        populate_row(line_data, headers, result);
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


// Used when reading in a csv, fits in the row data to its corresponding column in target.
// Assumes that row_data[i] corresponds to headers[i]. If len(row_data) < len(headers), the extra columns are filled in as null. If len(row_data) > len(headers), the extra data is left out.
// Also converts the strings into Table_Entry's along the way (i.e. converts them into floats or null values as needed).
void populate_row(const vector<string>& row_data, const vector<string>& headers, map<string, vector<Table_Entry>>& target) {
    for (unsigned int i = 0; i < headers.size(); i++) {
        if (i < row_data.size())
            target.at(headers[i]).push_back(convert_string_to_table_entry(row_data[i]));
        else
            target.at(headers[i]).push_back(convert_string_to_table_entry(""));
    }
}


Table_Entry convert_string_to_table_entry(const string& str) {
    variant<monostate, float, string> converted_val;
    if (str.empty()) {
        converted_val = monostate{};
    }
    else if (is_float(str)) {
        converted_val = stof(str);
    }
    else {
        converted_val = str;
    }
    return converted_val;
}


bool is_float(const string& str) {
    if (str.empty()) return false;

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


unsigned int get_day_of_year(const std::string& schedule_date_string, unsigned int year) {
    const string date_str = schedule_date_string + " " + to_string(year);
    tm t{};
    istringstream ss(date_str);
    ss >> get_time(&t, "%a  %b %d %Y");

    if (ss.fail()) {
        cerr << "Invalid date string provided: " << date_str << "\n";
        throw exception();
    }

    return t.tm_yday;
}