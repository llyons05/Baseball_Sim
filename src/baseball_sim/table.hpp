#pragma once

#include "baseball_exceptions.hpp"

#include <vector>
#include <string>
#include <map>
#include <iostream>
#include <stdexcept>

class Table_Row {
    public:

        Table_Row(){}
        Table_Row(std::map<std::string, std::string> row_data) {
            this->row_data = row_data;
        }

        std::string get_stat(std::string stat_name, std::string default_val);
        int get_stat(std::string stat_name, int default_val);
        float get_stat(std::string stat_name, float default_val);

        bool has_stat(std::string stat_name) {
            return row_data.find(stat_name) != row_data.end();
        }

        bool has_attributes(std::map<std::string, std::vector<std::string>> attributes);
        std::map<std::string, std::string> get_row_data() {
            return row_data;
        }

    private:
        std::string get_raw_stat_data(std::string stat_name) {
            if (has_stat(stat_name)) {
                return row_data[stat_name];
            }
            throw invalidStatNameException("Table row", "Unknown", stat_name, "");
        }

        std::map<std::string, std::string> row_data;
};


class Stat_Table {
    public:
        std::string stat_type;
        std::string stat_table_id;

        Stat_Table() {}
        Stat_Table(std::vector<std::map<std::string, std::string>> stat_table, std::string stat_type, std::string stat_table_id = "");


        int find_row(std::map<std::string, std::vector<std::string>> search_attributes);
        std::vector<Table_Row> filter_rows(std::map<std::string, std::vector<std::string>> search_attributes);
        std::vector<Table_Row> get_rows() {
            return table_rows;
        }

        template <class T>
        T get_stat(std::string stat_name, unsigned int row_index, T default_val) {
            if ((row_index < 0) || (row_index >= table_rows.size())) {
                std::cerr << "Invalid row index (" << row_index << ") when accessing stat " << stat_name << " in table " << stat_table_id << "\n";
                throw std::exception();
            }

            return table_rows[row_index].get_stat(stat_name, default_val);
        }

    private:
        std::vector<Table_Row> table_rows;
};