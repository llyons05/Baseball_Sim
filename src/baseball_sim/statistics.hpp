#pragma once

#include "baseball_exceptions.hpp"
#include "table.hpp"

#include <vector>
#include <map>
#include <string>
#include <stdexcept>


enum ePlayer_Stat_Types {
    PLAYER_BATTING,
    PLAYER_PITCHING,
    PLAYER_APPEARANCES,
    PLAYER_BASERUNNING,
    NUM_PLAYER_STAT_TYPES
};

extern std::string PLAYER_STAT_TYPES[NUM_PLAYER_STAT_TYPES];

class Player_Stats {

    public:
        std::string player_id;
        std::string cache_id;
        Stat_Table stat_tables[NUM_PLAYER_STAT_TYPES];
        int current_year;

        Player_Stats(){}

        Player_Stats(const std::string& player_id, int year, const std::string& team_abbreviation, Stat_Table player_stat_tables[NUM_PLAYER_STAT_TYPES]);

        template <class T>
        T get_stat(ePlayer_Stat_Types stat_type, const std::string& stat_name, const T& default_val) const {
            int row_index = current_table_row_indices[stat_type];
            return stat_tables[stat_type].get_stat(stat_name, row_index, default_val);
        }

    private:
        int current_table_row_indices[NUM_PLAYER_STAT_TYPES] = {0};
        std::string current_team_abbreviation;

        void change_stat_table_target_row(ePlayer_Stat_Types stat_type, int year, const std::string& team_abbreviation);
};


enum eTeam_Stat_Types {
    TEAM_ROSTER,
    TEAM_BATTING,
    TEAM_PITCHING,
    TEAM_COMMON_BATTING_ORDERS,
    TEAM_INFO,
    NUM_TEAM_STAT_TYPES
};


extern std::string TEAM_STAT_TYPES[NUM_TEAM_STAT_TYPES];

class Team_Stats {
    public:
        std::string team_name;
        std::string team_cache_id;
        Stat_Table stat_tables[NUM_TEAM_STAT_TYPES];

        Team_Stats() {}
        Team_Stats(const std::string& team_id, Stat_Table team_stat_tables[NUM_TEAM_STAT_TYPES], int year);

        Table_Row get_row(eTeam_Stat_Types stat_type, const std::map<std::string, std::vector<Table_Entry>>& row_attributes);

        template <class T>
        T get_stat(eTeam_Stat_Types stat_type, const std::map<std::string, std::vector<Table_Entry>>& row_attributes, const std::string& stat_name, const T& default_val) {
            int row_index = stat_tables[stat_type].find_row(row_attributes);
            return stat_tables[stat_type].get_stat(stat_name, row_index, default_val);
        }
};

extern std::map<eTeam_Stat_Types, std::vector<ePlayer_Stat_Types>> TEAM_TO_PLAYER_STAT_CORRESPONDENCE;


enum eLeague_Stat_Types {
    LEAGUE_BATTING,
    LEAGUE_PITCHING,
    NUM_LEAGUE_STAT_TYPES
};

class League_Stats {
    public:
        Stat_Table stat_tables[NUM_LEAGUE_STAT_TYPES];

        League_Stats() {}
        League_Stats(Stat_Table league_stat_tables[NUM_LEAGUE_STAT_TYPES]);

        Table_Row get_row(eLeague_Stat_Types stat_type, int year) {
            return stat_tables[stat_type].get_rows().at(get_row_index(year));
        }

        template <class T>
        T get_stat(eLeague_Stat_Types stat_type, int year, const std::string& stat_name, const T& default_val) {
            int row_index = get_row_index(year);
            return stat_tables[stat_type].get_stat(stat_name, row_index, default_val);
        }

    private:
        int start_year;

        int get_row_index(int year) {
            if (year > start_year) {
                return start_year;
            }
            return start_year - year;
        }
};

extern League_Stats LEAGUE_AVG_STATS;