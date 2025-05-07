#pragma once

#include "table.hpp"

#include <fstream>
#include <sstream>
#include <vector>
#include <map>
#include <variant>

bool file_exists(const std::string& filename);
std::ifstream open_file(const std::string& filename);
std::map<std::string, std::vector<Table_Entry>> read_csv_file(const std::string& filename);
std::vector<std::string> read_csv_line(const std::string& line);
void populate_row(const std::vector<std::string>& row_data, const std::vector<std::string>& headers, std::map<std::string, std::vector<Table_Entry>>& target);

Table_Entry convert_string_to_table_entry(const std::string& str);
bool is_float(const std::string& str);

std::string get_player_cache_id(const std::string& player_id, const std::string& team_abbreviation, unsigned int year);
std::string get_player_cache_id(const std::string& player_id, const std::string& team_cache_id);
std::string get_team_cache_id(const std::string& team_abbr, unsigned int year);

unsigned int get_day_of_year(const std::string& schedule_date_string, unsigned int year);