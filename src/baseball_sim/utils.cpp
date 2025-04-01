#include "utils.hpp"

#include <fstream>
#include <sstream>
#include <vector>
#include <map>
#include <iostream>

using namespace std;

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


vector<map<string, float>> convert_rows_to_float(const std::vector<std::map<std::string, std::string>>& rows) {
    vector<map<string, float>> result;
    for (const map<string, string>& row : rows) {
        result.push_back(convert_row_to_float(row));
    }
    return result;
}


map<string, float> convert_row_to_float(const map<string, string>& row) {
    map<string, float> result;
    for (auto const& [key, value] : row) {
        if (is_float(value)) {
            result.insert({key, stof(value)});
        }
    }
    return result;
}


bool is_float(const string& str) {
    if (str.empty())
        return false;

    char* ptr;
    strtof(str.c_str(), &ptr);
    return (*ptr) == '\0';
}