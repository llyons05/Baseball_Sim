#pragma once

#include "baseball_exceptions.hpp"
#include "utils.hpp"

#include <vector>
#include <string>
#include <map>
#include <iostream>
#include <stdexcept>
#include <variant>


typedef std::variant<std::string, float, std::monostate> Table_Entry;

class Table_Row {
    public:

        Table_Row(){}
        Table_Row(const std::map<std::string, Table_Entry>& row_data) {
            this->row_data = row_data;
        }


        bool has_stat(const std::string& stat_name) const {
            return row_data.find(stat_name) != row_data.end();
        }


        Table_Entry get_entry(const std::string& stat_name) const {
            if (has_stat(stat_name)) {
                return row_data.at(stat_name);
            }
            throw invalidStatNameException("Table row", "Unknown", stat_name, "");
        }


        template <class T>
        T get_stat(const std::string& stat_name, const T& default_val) const {
            const Table_Entry& entry = get_entry(stat_name);
            if (std::holds_alternative<std::monostate>(entry)) {
                return default_val;
            }
            return std::get<T>(entry);
        }


        /*
        Takes in a dictionary in the format: {"attribute_name": "expected_value", etc...}
        and checks to see if:
            1. The row has an attribute with that name,
            2. That the value associated with the attribute is equal to the expected value.

        If the dictionary that is passed is empty, it will return true.
        */
        bool has_attributes(const std::map<std::string, std::vector<Table_Entry>>& attributes) const {
            for (auto const& [attr_name, attr_values] : attributes) {
                bool found_attribute = false;
                if (has_stat(attr_name)) {
                    const Table_Entry& existing_value = get_entry(attr_name);
                    for (const Table_Entry& value : attr_values) {
                        if (value == existing_value) {
                            found_attribute = true;
                            break;
                        }
                    }
                }
                if (!found_attribute) return false;
            }
            return true;
        }

        std::map<std::string, Table_Entry> get_row_data() const {
            return row_data;
        }

    private:
        std::map<std::string, Table_Entry> row_data;
};


class Stat_Table {
    public:
        std::string stat_type;
        std::string stat_table_id;

        Stat_Table() {}
        Stat_Table(const std::vector<std::map<std::string, std::string>>& stat_table, const std::string& stat_type, const std::string& stat_table_id = "") {
            this->stat_type = stat_type;
            this->stat_table_id = stat_table_id;

            for (const std::map<std::string, Table_Entry>& row : convert_rows_to_table_entries(stat_table)) {
                this->table_rows.push_back(Table_Row(row));
            }
        }


        int find_row(const std::map<std::string, std::vector<Table_Entry>>& search_attributes) const {
            for (unsigned int i = 0; i < table_rows.size(); i++) {
                if (table_rows[i].has_attributes(search_attributes)) {
                    return i;
                }
            }
            return -1;
        }


        std::vector<Table_Row> filter_rows(const std::map<std::string, std::vector<Table_Entry>>& search_attributes) const {
            std::vector<Table_Row> result;
            for (const Table_Row& row : table_rows) {
                if (row.has_attributes(search_attributes)) result.push_back(row);
            }
            return result;
        }


        std::vector<Table_Row> get_rows() const {
            return table_rows;
        }


        template <class T>
        T get_stat(const std::string& stat_name, unsigned int row_index, T default_val) const {
            if ((row_index < 0) || (row_index >= table_rows.size())) {
                std::cerr << "Invalid row index (" << row_index << ") when accessing stat " << stat_name << " in table " << stat_table_id << "\n";
                throw std::exception();
            }

            return table_rows[row_index].get_stat(stat_name, default_val);
        }

    private:
        std::vector<Table_Row> table_rows;
};