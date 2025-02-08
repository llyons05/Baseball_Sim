#include "load_stats.hpp"
#include "baseball_game.hpp"

#include <iostream>
#include <string>
#include <random>
#include <cstdint>
#include <time.h>


int main() {
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

    Team home_team = loader.load_team(home_team_name, home_team_year);
    Team away_team = loader.load_team(away_team_name, away_team_year);

    int total_wins[2] = {0};
    int total_runs[2] = {0};

    home_team.print_fielders();
    away_team.print_fielders();

    home_team.print_batting_order();
    away_team.print_batting_order();

    for (int i = 0; i < num_games; i++) {
        Baseball_Game game(home_team, away_team);
        Game_Result result = game.play_game();

        if (result.final_score[HOME_TEAM] > result.final_score[AWAY_TEAM]) total_wins[HOME_TEAM]++;
        else if (result.final_score[HOME_TEAM] < result.final_score[AWAY_TEAM]) total_wins[AWAY_TEAM]++;

        total_runs[HOME_TEAM] += result.final_score[HOME_TEAM];
        total_runs[AWAY_TEAM] += result.final_score[AWAY_TEAM];

        home_team.reset();
        away_team.reset();
    }

    std::cout << "Result of " << num_games << " games:\n";
    std::cout << home_team.team_name << ": " << total_wins[HOME_TEAM] << " wins\t Total runs: " << total_runs[HOME_TEAM] << "\n";
    std::cout << away_team.team_name << ": " << total_wins[AWAY_TEAM] << " wins\t Total runs: " << total_runs[AWAY_TEAM] << "\n";
}