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
void play_single_game();
void play_season();

int main() {
    debug_print("IN DEBUG MODE\n");
    game_viewer_print("IN VIEWING MODE\n");
    set_up_rand();

    std::string sim_type = get_simulation_type();

    if (sim_type == "m") play_single_game();
    else if (sim_type == "s") play_season();

    return 0;
}


void play_season() {
    Stat_Loader loader;

    unsigned int season_year = get_user_input<unsigned int>("Input season to simulate: ");
    unsigned int season_sims = get_user_input<unsigned int>("Input number of times to simulate season: ");

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


void play_single_game() {
    Stat_Loader loader;

    std::string team_1_name = get_user_input<std::string>("Input Team 1 Abbreviation (Ex: NYY or LAD): ");
    unsigned int team_1_year = get_user_input<unsigned int>("Input Team 1 Year (Ex: 1924 or 2024): ");
    std::string team_2_name = get_user_input<std::string>("Input Team 2 Abbreviation (Ex: NYY or LAD): ");
    unsigned int team_2_year = get_user_input<unsigned int>("Input Team 2 Year (Ex: 1924 or 2024): ");
    unsigned int num_games = get_user_input<unsigned int>("Input number of games in the series: ");

    std::chrono::steady_clock::time_point load_start = std::chrono::steady_clock::now();
    Team* team_1 = loader.load_team(team_1_name, team_1_year);
    Team* team_2 = loader.load_team(team_2_name, team_2_year);
    Team* teams[2] = {team_1, team_2};

    loader.load_league_year_stats(team_1_year);
    loader.load_league_year_stats(team_2_year);

    float load_duration = (std::chrono::steady_clock::now() - load_start).count()/(1e+9);
    std::cout << "Data loaded in " << load_duration << " seconds\n\n";

    int total_wins[2] = {0};
    int total_runs[2] = {0};

    std::cout << "Running " << num_games << " games... ";

    std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();

    for (unsigned int i = 0; i < num_games; i++) {
        teams[0]->prepare_for_game(0, true);
        teams[1]->prepare_for_game(0, true);

        int home_id = i%2;
        int away_id = (i+1)%2;
        Baseball_Game game(teams[home_id], teams[away_id], 0);
        Game_Result result = game.play_game();

        if (result.winner == HOME_TEAM) total_wins[home_id]++;
        else total_wins[away_id]++;

        total_runs[home_id] += result.final_score[HOME_TEAM];
        total_runs[away_id] += result.final_score[AWAY_TEAM];

        teams[0]->reset();
        teams[1]->reset();
    }

    float duration = (std::chrono::steady_clock::now() - begin).count()/(1e+9);
    std::cout << "Completed in " << duration << " seconds (" << num_games/duration << " games/s)\n\n";

    std::cout << "Result of " << num_games << " games:\n";
    std::cout << team_1->team_name << ": " << total_wins[0] << " wins\t Total runs: " << total_runs[0] << "\n";
    std::cout << team_2->team_name << ": " << total_wins[1] << " wins\t Total runs: " << total_runs[1] << "\n\n";
    global_stats.print(num_games);
}