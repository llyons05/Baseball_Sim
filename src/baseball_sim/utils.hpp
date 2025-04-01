#pragma once

#include <fstream>
#include <sstream>
#include <vector>
#include <map>

std::ifstream open_file(const std::string& filename);
std::vector<std::map<std::string, std::string>> read_csv_file(const std::string& filename);
std::vector<std::string> read_csv_line(const std::string& line);
std::map<std::string, std::string> match_keys_to_values(const std::vector<std::string>& keys, const std::vector<std::string>& values);
std::vector<std::map<std::string, float>> convert_rows_to_float(const std::vector<std::map<std::string, std::string>>& rows);
std::map<std::string, float> convert_row_to_float(const std::map<std::string, std::string>& row);
bool is_float(const std::string& str);