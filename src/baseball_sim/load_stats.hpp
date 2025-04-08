#pragma once

#include "statistics.hpp"
#include "team.hpp"
#include "table.hpp"

#include <vector>
#include <string>
#include <fstream>
#include <sstream>
#include <map>
#include <unordered_map>
#include <memory>
#include <stdexcept>


const std::vector<std::string> PITCHING_POSITION_NAMES = {"P", "CL", "SP", "RP"};
const std::string DEFAULT_POSITION = "P";

extern std::unordered_map<std::string, std::shared_ptr<Player>> player_cache;
extern std::unordered_map<std::string, std::shared_ptr<Team>> team_cache;

class Stat_Loader {
    
    public:
        Stat_Loader() {}

        Team* load_team(const std::string& team_abbreviation, int year);
        Player* load_player(const std::string& player_name, const std::string& player_id, int year, const std::string& team_abbreviation, const std::vector<ePlayer_Stat_Types>& stats_to_load);
        void load_league_avgs();

    private:
        const std::string DATABASE_FILE_PATH = "../stat_collection/data";
        const std::string PLAYERS_FILE_PATH = DATABASE_FILE_PATH + "/players";
        const std::string TEAMS_FILE_PATH = DATABASE_FILE_PATH + "/teams";
        const std::string LEAGUE_FILE_PATH = DATABASE_FILE_PATH + "/league";

        std::string get_player_data_file_path(const std::string& player_id, const std::string& stat_type);
        std::string get_team_data_file_path(const std::string& team_abbreviation, int year, const std::string& team_data_file_type);
        std::string get_team_year_dir_path(const std::string& team_abbreviation, int year);
        std::string get_league_data_file_path(const std::string& stat_type);

        std::string get_player_cache_id(const std::string& player_id, const std::string& team_abbreviation, int year);
        bool is_player_cached(const std::string& player_cache_id);
        bool is_team_cached(const std::string& team_cache_id);
        Player* cache_player(const Player& player, const std::string& cache_id);
        Team* cache_team(const Team& team, const std::string& cache_id);

        Team_Stats load_team_stats(const std::string& team_abbreviation, int year);
        std::vector<Player*> load_team_roster(Team_Stats& team_stats, int year);
        Player_Stats load_necessary_player_stats(const std::string& player_id, int year, const std::string& team_abbreviation, const std::vector<ePlayer_Stat_Types>& stats_to_load);
        Stat_Table load_player_stat_table(const std::string& player_id, const std::string& team_abbreviation, int year, ePlayer_Stat_Types player_stat_type);
};