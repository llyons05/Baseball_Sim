#include "baseball_game.hpp"

#include "game_states.hpp"

Baseball_Game::Baseball_Game (Team home_team, Team away_team) {
    teams[HOME_TEAM] = home_team;
    teams[AWAY_TEAM] = away_team;

    score[HOME_TEAM] = 0;
    score[AWAY_TEAM] = 0;

    team_batting = AWAY_TEAM;
    half_inning = 0;
}

Game_Result Baseball_Game::play_game() {
    for (int i = 0; i <= MAX_HALF_INNINGS; i++) {
        int runs_scored = play_half_inning();
        score[team_batting] += runs_scored;
        team_batting = !team_batting;
    }
}

int Baseball_Game::play_half_inning() {
    Half_Inning inning = Half_Inning();
}