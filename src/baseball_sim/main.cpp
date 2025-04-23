#include "load_stats.hpp"
#include "baseball_game.hpp"
#include "probability.hpp"

#include <iostream>
#include <string>
#include <random>
#include <cstdint>
#include <time.h>
#include <chrono>


std::string get_simulation_type();
void play_single_game();
void play_season();

int main() {
    #if BASEBALL_DEBUG
        std::cout << "IN DEBUG MODE\n";
    #endif
    #if BASEBALL_VIEW
        std::cout << "IN VIEWING MODE\n";
    #endif
    set_up_rand();

    std::string sim_type = get_simulation_type();

    if (sim_type == "m") play_single_game();
    else if (sim_type == "s") play_season();
    
    return 0;
}


std::string get_simulation_type() {
    std::string sim_type = "";
    while ((sim_type != "m") && (sim_type != "s")) {
        std::cout << "Simulate single matchup or full season? (m/s): ";
        std::cin >> sim_type;
        std::cout << "\n";
    }

    return sim_type;
}


void play_season() {
    Stat_Loader loader;

    unsigned int season_year;
    std::cout << "Input season to simulate: ";
    std::cin >> season_year;
    std::cout << "\n";

    unsigned int sims_per_game;
    std::cout << "Input simulations per game: ";
    std::cin >> sims_per_game;
    std::cout << "\n";

    std::chrono::steady_clock::time_point load_start = std::chrono::steady_clock::now();
    
    std::cout << "Loading " << season_year << " Season, could take up to a minute...\n";
    Season season = loader.load_season(season_year);

    float load_duration = (std::chrono::steady_clock::now() - load_start).count()/(1e+9);
    std::cout << "Data loaded in " << load_duration << " seconds\n\n";

    std::vector<Team*> final_standings = season.run_games(sims_per_game);
    std::cout << "FINAL STANDINGS:\n";
    for (unsigned int i = 0; i < final_standings.size(); i++) {
        std::cout << "\t" << i+1 << ": " << final_standings[i]->team_stats.year_specific_abbreviation << "\t";
        std::cout << final_standings[i]->wins << "-" << final_standings[i]->losses << "\n";
    }
}


void play_single_game() {
    Stat_Loader loader;

    int num_games;
    std::string team_1_name;
    std::string team_2_name;
    int team_1_year;
    int team_2_year;
    
    std::cout << "Input Team 1 Abbreviation (Ex: NYY or LAD): ";
    std::cin >> team_1_name;
    std::cout << "\n";

    std::cout << "Input Team 1 Year (Ex: 1924 or 2024): ";
    std::cin >> team_1_year;
    std::cout << "\n";

    std::cout << "Input Team 2 Abbreviation (Ex: NYY or LAD): ";
    std::cin >> team_2_name;
    std::cout << "\n";

    std::cout << "Input Team 2 Year (Ex: 1924 or 2024): ";
    std::cin >> team_2_year;
    std::cout << "\n";

    std::cout << "Input number of games in the series: ";
    std::cin >> num_games;
    std::cout << "\n";

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

    #if BASEBALL_DEBUG || BASEBALL_VIEW
    std::cout << "\n";
    team_1->print_fielders();
    team_2->print_fielders();

    team_1->print_batting_order();
    team_2->print_batting_order();
    #endif

    std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();

    for (int i = 0; i < num_games; i++) {
        int home_id = i%2;
        int away_id = (i+1)%2;
        Baseball_Game game(teams[home_id], teams[away_id], 0);
        Game_Result result = game.play_game();

        if (result.final_score[HOME_TEAM] > result.final_score[AWAY_TEAM]) total_wins[home_id]++;
        else if (result.final_score[HOME_TEAM] < result.final_score[AWAY_TEAM]) total_wins[away_id]++;

        total_runs[home_id] += result.final_score[HOME_TEAM];
        total_runs[away_id] += result.final_score[AWAY_TEAM];

        teams[0]->reset();
        teams[1]->reset();
    }

    float duration = (std::chrono::steady_clock::now() - begin).count()/(1e+9);
    std::cout << "Completed in " << duration << " seconds (" << num_games/duration << " games/s)\n\n";

    std::cout << "Result of " << num_games << " games:\n";
    std::cout << team_1->team_name << ": " << total_wins[0] << " wins\t Total runs: " << total_runs[0] << "\n";
    std::cout << team_2->team_name << ": " << total_wins[1] << " wins\t Total runs: " << total_runs[1] << "\n";
}