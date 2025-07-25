#pragma once

#include "includes.hpp"

#include <vector>
#include <string>
#include <map>
#include <iostream>
#include <stdexcept>
#include <variant>


typedef std::variant<std::monostate, float, std::string> Table_Entry;

class Stat_Table {
    public:
        std::string stat_table_id;

        Stat_Table() {}
        Stat_Table(const std::map<std::string, std::vector<Table_Entry>>& table_data, const std::string& stat_table_id = "") {
            if (!is_table_data_valid(table_data)) {
                std::cerr << "ERROR: ROWS OF MISMATCHED SIZES IN TABLE " << stat_table_id << "\n";
                throw std::exception();
            }

            this->stat_table_id = stat_table_id;
            this->table_data = table_data;

            if (this->table_data.size() == 0) {
                column_size = 0;
            }
            else {
                column_size = this->table_data.begin()->second.size();
            }
        }

        // Return the index of the row with the given attributes, return -1 if no row exists with the given attributes.
        int find_row(const std::map<std::string, std::vector<Table_Entry>>& search_attributes) const {
            for (size_t i = 0; i < size(); i++) {
                if (row_has_attributes(i, search_attributes)) {
                    return i;
                }
            }
            return -1;
        }


        /* Return a vector of row indexes corresponding to rows with the given attributes. If search attributes is empty, returns all rows. */
        std::vector<size_t> filter_rows(const std::map<std::string, std::vector<Table_Entry>>& search_attributes) const {
            std::vector<size_t> result;
            for (size_t i = 0; i < size(); i++) {
                if (row_has_attributes(i, search_attributes)) result.push_back(i);
            }
            return result;
        }


        template <class T>
        T get_stat(const std::string& stat_name, size_t row_index, const T& default_val) const {
            if (row_index >= size()) {
                std::cerr << "Invalid row index (" << row_index << ") when accessing stat " << stat_name << " in table " << stat_table_id << "\n";
                throw std::exception();
            }

            return convert_entry(get_entry(row_index, stat_name), default_val);
        }


        template <class T>
        std::vector<T> column(const std::string& stat_name, const T& default_val) const {
            const std::vector<Table_Entry>& table_col = column(stat_name);
            std::vector<T> result;
            result.reserve(table_col.size());

            for (const Table_Entry& entry : table_col) {
                result.push_back(convert_entry(entry, default_val));
            }
            return result;
        }


        bool has_stat(const std::string& stat_name) const {
            return table_data.find(stat_name) != table_data.end();
        }


        size_t size() const {
            return column_size;
        }

        bool empty() const {
            return column_size == 0;
        }

    private:
        std::map<std::string, std::vector<Table_Entry>> table_data;
        size_t column_size = 0;

        const std::vector<Table_Entry>& column(const std::string& stat_name) const {
            try {
                return table_data.at(stat_name);
            }
            catch (const std::out_of_range&) {
                std::cerr << "Stat " + stat_name + " is not a column in table " + stat_table_id + "\n";
                throw std::out_of_range("");
            }
        }

        const Table_Entry& get_entry(size_t row, const std::string& column) const {
            try {
                return table_data.at(column).at(row);
            }
            catch (const std::out_of_range&) {
                std::cerr << "Stat " + column + " not in table " + stat_table_id + "at row " << row << "\n";
                throw std::out_of_range("");
            }
        }

        bool row_has_attributes(size_t row, const std::map<std::string, std::vector<Table_Entry>>& attributes) const {
            for (auto const& [attr_name, attr_values] : attributes) {
                bool found_attribute = false;
                if (has_stat(attr_name)) {
                    const Table_Entry& existing_value = get_entry(row, attr_name);
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

        bool is_table_data_valid(const std::map<std::string, std::vector<Table_Entry>>& data) const {
            if (data.size() == 0) { // Empty tables are ok
                return true;
            }
            size_t first_col_size = data.begin()->second.size();
            for (const auto&[header, column] : data) {
                if (column.size() != first_col_size) { // then we have columns of mismatched sizes, this is not ok
                    return false;
                }
            }
            return true;
        }


        template <class T>
        T convert_entry(const Table_Entry& entry, const T& default_val) const {
            if (std::holds_alternative<std::monostate>(entry)) {
                return default_val;
            }
            try {
                return std::get<T>(entry);
            }
            catch (const std::bad_variant_access&) {
                std::cerr << "Bad variant access in " << stat_table_id << " (default val was " << default_val << ")\n";
                throw std::exception();
            }
        }
};

// Container for multiple stat_tables
template <class Stat_Type, uint num_stat_types> // Change this to size_t maybe???
class Stat_Table_Container {
    public:
        Stat_Table stat_tables[num_stat_types];

        Stat_Table_Container(): stat_tables() {};
        Stat_Table_Container(Stat_Table stat_tables[num_stat_types]) {
            for (uint i = 0; i < num_stat_types; i++) {
                this->stat_tables[i] = stat_tables[i];
            }
        }

        template <class T>
        T get_stat(Stat_Type stat_type, const std::string& stat_name, size_t row_index, const T& default_val) const {
            return stat_tables[stat_type].get_stat(stat_name, row_index, default_val);
        }

        const Stat_Table& operator[](Stat_Type stat_type) const {
            if ((stat_type < 0) || (stat_type >= num_stat_types)) {
                throw std::out_of_range("Illegal stat_table access in Table_Container\n");
            }
            return stat_tables[stat_type];
        }
};