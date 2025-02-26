#include "load_stats.hpp"
#include "baseball_game.hpp"

#include <iostream>
#include <string>
#include <random>
#include <cstdint>
#include <time.h>
#include <chrono>


int main() {
    #if BASEBALL_DEBUG
        std::cout << "IN DEBUG MODE\n";
    #endif

    Stat_Loader loader;
    loader.load_league_avgs();
    srand(time(NULL));

    int num_games;
    std::string home_team_name;
    std::string away_team_name;
    int home_team_year;
    int away_team_year;
    
    std::cout << "Input Home Team Abbreviation (Ex: NYY or LAD): ";
    std::cin >> home_team_name;
    std::cout << "\n";

    std::cout << "Input Home Team Year (Ex: 1924 or 2024): ";
    std::cin >> home_team_year;
    std::cout << "\n";

    std::cout << "Input Away Team Abbreviation (Ex: NYY or LAD): ";
    std::cin >> away_team_name;
    std::cout << "\n";

    std::cout << "Input Away Team Year (Ex: 1924 or 2024): ";
    std::cin >> away_team_year;
    std::cout << "\n";

    std::cout << "Input number of games in the series: ";
    std::cin >> num_games;
    std::cout << "\n";

    std::chrono::steady_clock::time_point load_start = std::chrono::steady_clock::now();
    Team home_team = loader.load_team(home_team_name, home_team_year);
    Team away_team = loader.load_team(away_team_name, away_team_year);
    float load_duration = (std::chrono::steady_clock::now() - load_start).count()/(1e+9);
    std::cout << "Data loaded in " << load_duration << " seconds\n\n";

    Baseball_Game game(home_team, away_team);

    int total_wins[2] = {0};
    int total_runs[2] = {0};

    std::cout << "Running " << num_games << " games... ";

    #if BASEBALL_DEBUG
    std::cout << "\n";
    home_team.print_fielders();
    away_team.print_fielders();

    home_team.print_batting_order();
    away_team.print_batting_order();
    #endif

    std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();

    for (int i = 0; i < num_games; i++) {
        Game_Result result = game.play_game();

        if (result.final_score[HOME_TEAM] > result.final_score[AWAY_TEAM]) total_wins[HOME_TEAM]++;
        else if (result.final_score[HOME_TEAM] < result.final_score[AWAY_TEAM]) total_wins[AWAY_TEAM]++;

        total_runs[HOME_TEAM] += result.final_score[HOME_TEAM];
        total_runs[AWAY_TEAM] += result.final_score[AWAY_TEAM];

        game.reset();
    }

    float duration = (std::chrono::steady_clock::now() - begin).count()/(1e+9);
    std::cout << "Completed in " << duration << " seconds (" << num_games/duration << " games/s)\n\n";

    std::cout << "Result of " << num_games << " games:\n";
    std::cout << home_team.team_name << ": " << total_wins[HOME_TEAM] << " wins\t Total runs: " << total_runs[HOME_TEAM] << "\n";
    std::cout << away_team.team_name << ": " << total_wins[AWAY_TEAM] << " wins\t Total runs: " << total_runs[AWAY_TEAM] << "\n";
}