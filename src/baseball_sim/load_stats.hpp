#pragma once

#include "statistics.hpp"
#include "team.hpp"
#include "table.hpp"
#include "season.hpp"

#include <vector>
#include <string>
#include <fstream>
#include <sstream>
#include <map>
#include <unordered_map>
#include <memory>
#include <stdexcept>


class Stat_Loader {
    
    public:
        Stat_Loader() {}

        Team* load_team(const std::string& main_team_abbreviation, uint year);
        Player* load_player(const std::string& player_name, const std::string& player_id, uint year, const std::string& team_abbreviation, const std::vector<ePlayer_Stat_Types>& stats_to_load);
        void load_league_year_stats(uint year);
        Season load_season(uint year);

    private:
        const std::string RESOURCES_FILE_PATH = "../stat_collection/resources";
        const std::string DATABASE_FILE_PATH = "../stat_collection/data";
        const std::string PLAYERS_FILE_PATH = DATABASE_FILE_PATH + "/players";
        const std::string TEAMS_FILE_PATH = DATABASE_FILE_PATH + "/teams";
        const std::string LEAGUE_FILE_PATH = DATABASE_FILE_PATH + "/league";

        std::string get_player_data_file_path(const std::string& player_id, const std::string& stat_type);
        std::string get_team_data_file_path(const std::string& main_team_abbreviation, uint year, const std::string& team_data_file_type);
        std::string get_team_year_dir_path(const std::string& main_team_abbreviation, uint year);
        std::string get_league_data_file_path(const std::string& league_data_file_type, uint year);
        std::string get_league_year_dir_path(uint year);

        bool is_player_cached(const std::string& player_cache_id);
        bool is_team_cached(const std::string& team_cache_id);
        Player* cache_player(const Player& player, const std::string& cache_id);
        Team* cache_team(const Team& team, const std::string& cache_id);

        Team_Stats load_team_stats(const std::string& main_team_abbreviation, uint year);
        std::vector<Player*> load_team_roster(Team_Stats& team_stats, uint year);
        Player_Stats load_necessary_player_stats(const std::string& player_id, uint year, const std::string& team_abbreviation, const std::vector<ePlayer_Stat_Types>& stats_to_load);
        std::vector<ePlayer_Stat_Types> get_player_stat_types_to_load(const std::string& player_id, Team_Stats& team_stats);
        bool should_load_player_stat_type(const Team_Stats& team_stats, size_t player_row, ePlayer_Stat_Types stat_type);
        Stat_Table load_player_stat_table(const std::string& player_id, const std::string& team_abbreviation, uint year, ePlayer_Stat_Types player_stat_type);

        Stat_Table load_all_teams_table();
        std::vector<Team*> load_all_saved_teams_from_year(uint year);
        std::vector<std::string> load_all_real_team_abbrs_from_year(uint year);
};