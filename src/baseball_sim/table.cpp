#include "table.hpp"

#include <vector>
#include <string>
#include <map>
#include <iostream>

using namespace std;

Stat_Table::Stat_Table(vector<map<string, string>> stat_table, string stat_type, string stat_table_id) {
    this->stat_type = stat_type;
    this->stat_table_id = stat_table_id;

    for (map<string, string> row_data : stat_table) {
        Table_Row row(row_data);
        this->table_rows.push_back(row);
    }
}


/* Gets the index of the first row that matches the search attributes. If no rows meet the criteria, -1 is returned. */
int Stat_Table::find_row(map<string, vector<string>> search_attributes) {
    for (unsigned int i = 0; i < table_rows.size(); i++) {
        if (table_rows[i].has_attributes(search_attributes)) {
            return i;
        }
    }
    return -1;
}


vector<Table_Row> Stat_Table::filter_rows(map<string, vector<string>> search_attributes) {
    vector<Table_Row> result;
    for (Table_Row row : table_rows) {
        if (row.has_attributes(search_attributes)) result.push_back(row);
    }
    return result;
}


string Table_Row::get_stat(string stat_name, string default_val) {
    string raw_stat_data = get_raw_stat_data(stat_name);
    if (raw_stat_data.empty()) {
        raw_stat_data = default_val;
    }
    return raw_stat_data;
}


int Table_Row::get_stat(string stat_name, int default_val) {
    string raw_stat_data = get_raw_stat_data(stat_name);
    int stat_value;

    if (raw_stat_data.empty()) {
        stat_value = default_val;
    }
    else stat_value = stoi(raw_stat_data);
    return stat_value;
}


float Table_Row::get_stat(string stat_name, float default_val) {
    string raw_stat_data = get_raw_stat_data(stat_name);
    float stat_value;

    if (raw_stat_data.empty()) {
        stat_value = default_val;
    }
    else stat_value = stof(raw_stat_data);
    return stat_value;
}


/*
Takes in a dictionary in the format: {"attribute_name": "expected_value", etc...}
and checks to see if:
    1. The row has an attribute with that name,
    2. That the value associated with the attribute is equal to the expected value.

If the dictionary that is passed is empty, it will return true.
*/
bool Table_Row::has_attributes(map<string, vector<string>> attributes) {
    for (auto const& [attr_name, attr_values] : attributes) {
        bool found_attribute = false;
        for (string value : attr_values) {
            if (has_stat(attr_name) && (get_stat(attr_name, "") == value)) {
                found_attribute = true;
                break;
            }
        }
        if (!found_attribute) return false;
    }
    return true;
}