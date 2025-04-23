#include "baseball_game.hpp"

#include "game_states.hpp"

#include <iostream>
#include <random>
#include <time.h>

Baseball_Game::Baseball_Game(Team* home_team, Team* away_team, unsigned int day_of_year) {
    teams[HOME_TEAM] = home_team;
    teams[AWAY_TEAM] = away_team;

    score[HOME_TEAM] = 0;
    score[AWAY_TEAM] = 0;

    team_batting = AWAY_TEAM;
    half_inning_count = 0;

    this->day_of_year = day_of_year;
}


Game_Result Baseball_Game::play_game() {
    while (half_inning_count < MAX_HALF_INNINGS-1) {
        int runs_scored = play_half_inning();
        score[team_batting] += runs_scored;
        team_batting = !team_batting;
    }

    if (score[HOME_TEAM] <= score[AWAY_TEAM]) {
        while ((half_inning_count%2 == 1) || (score[HOME_TEAM] == score[AWAY_TEAM])) {
            int runs_scored = play_half_inning();
            score[team_batting] += runs_scored;
            team_batting = !team_batting;
        }
    }

    #if BASEBALL_VIEW
    print_game_result();
    #endif

    return Game_Result(teams[HOME_TEAM], teams[AWAY_TEAM], score, half_inning_count);
}


int Baseball_Game::play_half_inning() {
    Half_Inning inning(teams[team_batting], teams[!team_batting], half_inning_count);
    int runs_scored = inning.play();
    half_inning_count++;
    return runs_scored;
}


void Baseball_Game::print_game_result() {
    if (score[HOME_TEAM] > score[AWAY_TEAM]) {
        std::cout << teams[HOME_TEAM]->team_name << " wins!" << "\n";
    }
    else {
        std::cout << teams[AWAY_TEAM]->team_name << " wins!" << "\n";
    }
    std::cout << "TOTAL INNINGS: " << (float)half_inning_count/2 << "\n";
    std::cout << "FINAL SCORE: " << score[HOME_TEAM] << " - " << score[AWAY_TEAM] << "\n\n";
}