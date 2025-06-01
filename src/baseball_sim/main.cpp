#include "includes.hpp"
#include "load_stats.hpp"
#include "baseball_game.hpp"
#include "probability.hpp"
#include "user_interface.hpp"

#include <iostream>
#include <iomanip>
#include <string>
#include <random>
#include <cstdint>
#include <time.h>
#include <chrono>


std::string get_simulation_type();
void play_series();
void play_season();

int main() {
    debug_print("IN DEBUG MODE\n");
    game_viewer_print("IN VIEWING MODE\n");
    set_up_rand();

    std::string sim_type = get_simulation_type();

    if (sim_type == "t") play_series();
    else if (sim_type == "s") play_season();

    return 0;
}


void play_season() {
    Stat_Loader loader;

    uint season_year = get_user_input<uint>("Input season to simulate: ");
    uint season_sims = get_user_input<uint>("Input number of times to simulate season: ");

    std::chrono::steady_clock::time_point load_start = std::chrono::steady_clock::now();
    
    std::cout << "Loading " << season_year << " Season, could take up to a minute...\n";
    Season season = loader.load_season(season_year);

    float load_duration = (std::chrono::steady_clock::now() - load_start).count()/(1e+9);
    std::cout << "Data loaded in " << load_duration << " seconds\n\n";

    std::chrono::steady_clock::time_point sim_start = std::chrono::steady_clock::now();

    std::cout << "Simulating " << season_year << " season " << season_sims << " times...";
    std::vector<Team*> final_standings = season.run_games(season_sims);

    float sim_duration = (std::chrono::steady_clock::now() - sim_start).count()/(1e+9);
    std::cout << " Completed in " << sim_duration << " seconds\n\n";

    std::cout << "FINAL STANDINGS:\n";
    std::cout << "RANK\tTEAM\tW-L\t\tR-RA\n";
    float total_runs = 0;
    for (size_t i = 0; i < final_standings.size(); i++) {
        float wins = (float)final_standings[i]->wins / season_sims;
        float losses = (float)final_standings[i]->losses / season_sims;
        float runs_scored = (float)final_standings[i]->runs_scored/(season_sims*final_standings[i]->team_stats[TEAM_SCHEDULE].size());
        float runs_allowed = (float)final_standings[i]->runs_allowed/(season_sims*final_standings[i]->team_stats[TEAM_SCHEDULE].size());
        total_runs += runs_scored;

        std::cout << "    " << i+1 << ":\t" << final_standings[i]->team_stats.year_specific_abbreviation << "\t";
        std::cout << std::fixed << std::setprecision(1) << wins << "-" << losses << "\t";
        std::cout << runs_scored << "-" << runs_allowed << "\n";
    }
    std::cout << "AVG:\t\t\t\t" << total_runs/final_standings.size() << "-" << total_runs/final_standings.size() << "\n\n";
    global_stats.print(season_sims);
}


void play_series() {
    Stat_Loader loader;

    std::string home_team_name = get_user_input<std::string>("Input Home Team Abbreviation (Ex: NYY or LAD): ");
    uint home_team_year = get_user_input<uint>("Input Home Team Year (Ex: 1924 or 2024): ");
    std::string away_team_name = get_user_input<std::string>("Input Away Team Abbreviation (Ex: NYY or LAD): ");
    uint away_team_year = get_user_input<uint>("Input Away Team Year (Ex: 1924 or 2024): ");
    uint num_games = get_user_input<uint>("Input number of games in the series (ex: the world series is a 7 game series): ");
    uint num_sims = get_user_input<uint>("Input number of times to simulate series: ");
    bool swap_teams = num_games > 4;

    std::chrono::steady_clock::time_point load_start = std::chrono::steady_clock::now();
    Team* home_team = loader.load_team(home_team_name, home_team_year);
    Team* away_team = loader.load_team(away_team_name, away_team_year);

    loader.load_league_year_stats(home_team_year);
    loader.load_league_year_stats(away_team_year);

    Series series(home_team, away_team, num_games, num_sims, swap_teams);

    float load_duration = (std::chrono::steady_clock::now() - load_start).count()/(1e+9);
    std::cout << "Data loaded in " << load_duration << " seconds\n\n";

    std::cout << "Running ~" << num_games*num_sims << " games... ";

    std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();

    series.play();

    float duration = (std::chrono::steady_clock::now() - begin).count()/(1e+9);
    std::cout << "Completed in " << duration << " seconds (" << series.total_games_played/duration << " games/s)\n\n";

    series.print_results();
    global_stats.print(num_sims);
}