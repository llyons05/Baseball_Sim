#pragma once

#include <vector>
#include <string>
#include <map>
#include <iostream>
#include <stdexcept>
#include <variant>
#include <tuple>


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


        int find_row(const std::map<std::string, std::vector<Table_Entry>>& search_attributes) const {
            for (unsigned int i = 0; i < size(); i++) {
                if (row_has_attributes(i, search_attributes)) {
                    return i;
                }
            }
            return -1;
        }


        /* Return a vector of row indexes corresponding to rows with the given attributes. If search attributes is empty, returns all rows. */
        std::vector<unsigned int> filter_rows(const std::map<std::string, std::vector<Table_Entry>>& search_attributes) const {
            std::vector<unsigned int> result;
            for (unsigned int i = 0; i < size(); i++) {
                if (row_has_attributes(i, search_attributes)) result.push_back(i);
            }
            return result;
        }


        template <class T>
        T get_stat(const std::string& stat_name, unsigned int row_index, const T& default_val) const {
            if (row_index >= size()) {
                std::cerr << "Invalid row index (" << row_index << ") when accessing stat " << stat_name << " in table " << stat_table_id << "\n";
                throw std::exception();
            }

            const Table_Entry& entry = get_entry(row_index, stat_name);
            if (std::holds_alternative<std::monostate>(entry)) {
                return default_val;
            }
            return std::get<T>(entry);
        }


        bool has_stat(const std::string& stat_name) const {
            return table_data.find(stat_name) != table_data.end();
        }


        unsigned int size() const {
            return column_size;
        }

    private:
        std::map<std::string, std::vector<Table_Entry>> table_data;
        unsigned int column_size = 0;

        const Table_Entry& get_entry(unsigned int row, const std::string& column) const {
            try {
                return table_data.at(column).at(row);
            }
            catch (const std::out_of_range&) {
                std::cerr << "Stat " + column + " not in table " + stat_table_id + "at row " << row << "\n";
                throw std::out_of_range("Stat " + column + " not in table " + stat_table_id + "\n");
            }
        }

        bool row_has_attributes(unsigned int row, const std::map<std::string, std::vector<Table_Entry>>& attributes) const {
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
            unsigned int first_col_size = data.begin()->second.size();
            for (const auto&[header, column] : data) {
                if (column.size() != first_col_size) { // then we have columns of mismatched sizes, this is not ok
                    return false;
                }
            }
            return true;
        }
};

// Container for multiple stat_tables
template <class Stat_Type, unsigned int num_stat_types>
class Stat_Table_Container {
    public:
        Stat_Table stat_tables[num_stat_types];

        Stat_Table_Container(): stat_tables() {};
        Stat_Table_Container(Stat_Table stat_tables[num_stat_types]) {
            for (unsigned int i = 0; i < num_stat_types; i++) {
                this->stat_tables[i] = stat_tables[i];
            }
        }

        template <class T>
        T get_stat(Stat_Type stat_type, const std::string& stat_name, unsigned int row_index, const T& default_val) const {
            return stat_tables[stat_type].get_stat(stat_name, row_index, default_val);
        }
};