#pragma once

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
    PLAYER_BATTING_AGAINST,
    PLAYER_PITCH_SUMMARY_BATTING,
    PLAYER_PITCH_SUMMARY_PITCHING,
    NUM_PLAYER_STAT_TYPES
};

extern std::string PLAYER_STAT_NAMES[NUM_PLAYER_STAT_TYPES];
extern std::map<ePlayer_Stat_Types, unsigned int> PLAYER_STAT_EARLIEST_YEARS;

class Player_Stats : public Stat_Table_Container<ePlayer_Stat_Types, NUM_PLAYER_STAT_TYPES> {
    public:
        std::string player_id;
        std::string cache_id;
        unsigned int current_year;

        Player_Stats(){}
        Player_Stats(const std::string& player_id, unsigned int year, const std::string& team_abbreviation, Stat_Table player_stat_tables[NUM_PLAYER_STAT_TYPES]);

        template <class T>
        T get_stat(ePlayer_Stat_Types stat_type, const std::string& stat_name, const T& default_val) const {
            size_t row_index = current_table_row_indices[stat_type];
            return stat_tables[stat_type].get_stat(stat_name, row_index, default_val);
        }

    private:
        size_t current_table_row_indices[NUM_PLAYER_STAT_TYPES] = {0};
        std::string current_team_abbreviation;

        void change_stat_table_target_row(ePlayer_Stat_Types stat_type, unsigned int year, const std::string& team_abbreviation);
};

bool is_player_stat_out_of_date(ePlayer_Stat_Types stat_type);

enum eTeam_Stat_Types {
    TEAM_ROSTER,
    TEAM_BATTING,
    TEAM_PITCHING,
    TEAM_COMMON_BATTING_ORDERS,
    TEAM_INFO,
    TEAM_SCHEDULE,
    NUM_TEAM_STAT_TYPES
};


extern std::string TEAM_STAT_NAMES[NUM_TEAM_STAT_TYPES];
extern std::map<eTeam_Stat_Types, std::vector<ePlayer_Stat_Types>> TEAM_TO_PLAYER_STAT_CORRESPONDENCE;

class Team_Stats : public Stat_Table_Container<eTeam_Stat_Types, NUM_TEAM_STAT_TYPES> {
    public:
        std::string main_team_abbreviation;
        std::string year_specific_abbreviation;
        std::string team_cache_id;
        unsigned int days_in_schedule;
        unsigned int year;

        Team_Stats() {}
        Team_Stats(const std::string& main_team_abbreviation, Stat_Table team_stat_tables[NUM_TEAM_STAT_TYPES], unsigned int year);
};


enum eLeague_Stat_Types {
    LEAGUE_BATTING,
    LEAGUE_PITCHING,
    LEAGUE_PITCH_SUMMARY_BATTING,
    LEAGUE_PITCH_SUMMARY_PITCHING,
    LEAGUE_STANDINGS,
    NUM_LEAGUE_STAT_TYPES
};

extern std::string LEAGUE_STAT_NAMES[NUM_LEAGUE_STAT_TYPES];
extern std::map<eLeague_Stat_Types, unsigned int> LEAGUE_STAT_EARLIEST_YEARS;

typedef Stat_Table_Container<eLeague_Stat_Types, NUM_LEAGUE_STAT_TYPES> League_Stats;

// Holds League Stats for all loaded years
class All_League_Stats_Wrapper {
    public:
        All_League_Stats_Wrapper(){}
        void add_year(unsigned int year, const League_Stats& year_table);
        bool holds_year(unsigned int year) const;
        const League_Stats& get_year(unsigned int year);
        unsigned int get_avg_pitcher_cooldown(unsigned int year);

        template <class T>
        T get_stat(eLeague_Stat_Types stat_type, unsigned int year, const std::string& stat_name, const T& default_val) const {
            return league_stat_tables.at(year).get_stat(stat_type, stat_name, 0, default_val);
        }

    private:
        // Keys are years, values are League Stats for that year
        std::map<unsigned int, League_Stats> league_stat_tables;
};


extern All_League_Stats_Wrapper ALL_LEAGUE_STATS;