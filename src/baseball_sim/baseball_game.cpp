#include "baseball_game.hpp"

#include "includes.hpp"
#include "game_states.hpp"

#include <iostream>
#include <random>
#include <time.h>

Baseball_Game::Baseball_Game(Team* home_team, Team* away_team, uint day_of_year) {
    teams[HOME_TEAM] = home_team;
    teams[AWAY_TEAM] = away_team;

    score[HOME_TEAM] = 0;
    score[AWAY_TEAM] = 0;

    team_batting = AWAY_TEAM;
    half_inning_count = 0;

    this->day_of_year = day_of_year;
}


Game_Result Baseball_Game::play_game() {
    game_viewer_line(
        std::cout << "MATCHUP: " + teams[AWAY_TEAM]->team_name + " @ " + teams[HOME_TEAM]->team_name + "\n";
        teams[AWAY_TEAM]->print_fielders();
        teams[HOME_TEAM]->print_fielders();

        teams[AWAY_TEAM]->print_batting_order();
        teams[HOME_TEAM]->print_batting_order();
    )

    // Play first 8.5 innings
    while (half_inning_count < MAX_HALF_INNINGS-1) {
        int runs_scored = play_half_inning();
        score[team_batting] += runs_scored;
        team_batting = !team_batting;
    }

    // keep playing innings until someone wins
    if (score[HOME_TEAM] <= score[AWAY_TEAM]) {
        while ((half_inning_count%2 == 1) || (score[HOME_TEAM] == score[AWAY_TEAM])) {
            int runs_scored = play_half_inning();
            score[team_batting] += runs_scored;
            team_batting = !team_batting;
        }
    }

    game_viewer_line(print_game_result());
    return Game_Result(teams[HOME_TEAM], teams[AWAY_TEAM], score, half_inning_count);
}


uint8_t Baseball_Game::play_half_inning() {
    game_viewer_print(teams[AWAY_TEAM]->team_name +"| " << score[AWAY_TEAM] <<"-"<< score[HOME_TEAM] << " |"+ teams[HOME_TEAM]->team_name + "\n");
    Half_Inning inning(teams[team_batting], teams[!team_batting], half_inning_count, day_of_year);
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