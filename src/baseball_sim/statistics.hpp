#pragma once

#include "table.hpp"
#include "includes.hpp"

#include <vector>
#include <map>
#include <string>
#include <stdexcept>
#include <cstdint>
#include <iostream>
#include <iomanip>

enum ePlayer_Stat_Types {
    PLAYER_BATTING,
    PLAYER_PITCHING,
    PLAYER_FIELDING,
    PLAYER_APPEARANCES,
    PLAYER_BASERUNNING,
    PLAYER_BASERUNNING_AGAINST,
    PLAYER_BATTING_AGAINST,
    NUM_PLAYER_STAT_TYPES
};

extern std::string PLAYER_STAT_NAMES[NUM_PLAYER_STAT_TYPES];
extern std::map<ePlayer_Stat_Types, uint> PLAYER_STAT_EARLIEST_YEARS;

class Player_Stats : public Stat_Table_Container<ePlayer_Stat_Types, NUM_PLAYER_STAT_TYPES> {
    public:
        std::string player_id;
        std::string cache_id;
        uint current_year;

        Player_Stats(){}
        Player_Stats(const std::string& player_id, uint year, const std::string& team_abbreviation, Stat_Table player_stat_tables[NUM_PLAYER_STAT_TYPES]);

        template <class T>
        T get_stat(ePlayer_Stat_Types stat_type, const std::string& stat_name, const T& default_val) const {
            size_t row_index = current_table_row_indices[stat_type];
            return stat_tables[stat_type].get_stat(stat_name, row_index, default_val);
        }

    private:
        size_t current_table_row_indices[NUM_PLAYER_STAT_TYPES] = {0};
        std::string current_team_abbreviation;

        void change_stat_table_target_row(ePlayer_Stat_Types stat_type, uint year, const std::string& team_abbreviation);
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
        uint days_in_schedule;
        uint year;

        Team_Stats() {}
        Team_Stats(const std::string& main_team_abbreviation, Stat_Table team_stat_tables[NUM_TEAM_STAT_TYPES], uint year);
    private:
        void set_days_in_schedule();
};


enum eLeague_Stat_Types {
    LEAGUE_BATTING,
    LEAGUE_PITCHING,
    LEAGUE_FIELDING,
    LEAGUE_BASERUNNING,
    LEAGUE_BATTING_BY_BASES,
    LEAGUE_STANDINGS,
    NUM_LEAGUE_STAT_TYPES
};

extern std::string LEAGUE_STAT_NAMES[NUM_LEAGUE_STAT_TYPES];
extern std::map<eLeague_Stat_Types, uint> LEAGUE_STAT_EARLIEST_YEARS;

class League_Stats : public Stat_Table_Container<eLeague_Stat_Types, NUM_LEAGUE_STAT_TYPES> {
    public:
        float at_bat_probs[NUM_AB_OUTCOMES];
        float hit_or_out_probs[2];
        float hit_type_probs[4];
        float steal_attempt_probs[2][2];
        float steal_success_probs[2][2];

        float sbo_on_first_percent;
        uint year;

        League_Stats() {}
        League_Stats(uint year, Stat_Table league_stat_tables[NUM_LEAGUE_STAT_TYPES]);

    private:
        void populate_probs();
        void populate_at_bat_probs();
        void populate_steal_probs();
};

// Holds League Stats for all loaded years
class All_League_Stats_Wrapper {
    public:
        All_League_Stats_Wrapper(){}
        void add_year(uint year, const League_Stats& year_table);
        bool holds_year(uint year) const;
        const League_Stats& get_year(uint year) const;

        template <class T>
        T get_stat(eLeague_Stat_Types stat_type, uint year, const std::string& stat_name, const T& default_val) const {
            return league_stat_tables.at(year).get_stat(stat_type, stat_name, 0, default_val);
        }

        const League_Stats& operator[](uint year) const {
            return get_year(year);
        }

    private:
        // Keys are years, values are League Stats for that year
        std::map<uint, League_Stats> league_stat_tables;
}
extern ALL_LEAGUE_STATS;


struct Global_Stat_Container {
    uint32_t balls_in_play = 0;
    uint32_t total_PAs = 0;
    uint32_t total_hits = 0;

    void print(int divisor = 1) {
        std::cout << std::fixed << std::setprecision(3);
        std::cout << "BALL IN PLAY%: " << ((float)balls_in_play)/(float)total_PAs << "\n"
                  << "         HITS: " << total_hits/divisor << "\n"
                  << "          PAs: " << total_PAs/divisor << "\n";
    }
}
extern global_stats;