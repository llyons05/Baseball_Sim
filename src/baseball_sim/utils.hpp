#pragma once

#include <fstream>
#include <sstream>
#include <vector>
#include <map>
#include <variant>

std::ifstream open_file(const std::string& filename);
std::vector<std::map<std::string, std::string>> read_csv_file(const std::string& filename);
std::vector<std::string> read_csv_line(const std::string& line);
std::map<std::string, std::string> match_keys_to_values(const std::vector<std::string>& keys, const std::vector<std::string>& values);

std::vector<std::map<std::string, std::variant<std::monostate, float, std::string>>> convert_rows_to_table_entries(const std::vector<std::map<std::string, std::string>>& rows);
std::map<std::string, std::variant<std::monostate, float, std::string>> convert_row_to_table_entry(const std::map<std::string, std::string>& row);
bool is_float(const std::string& str);

std::string get_player_cache_id(const std::string& player_id, const std::string& team_abbreviation, int year);
std::string get_player_cache_id(const std::string& player_id, const std::string& team_cache_id);
std::string get_team_cache_id(const std::string& team_abbr, int year);