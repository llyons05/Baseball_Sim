#pragma once

#include "player.hpp"
#include "team.hpp"
#include "baseball_game.hpp"

#include <vector>
#include <string>


class Matchup {
    public:
        Team* home_team;
        Team* away_team;
        uint day_of_year;

        uint times_played = 0;
        uint runs_scored[2]{0};
        uint games_won[2]{0};

        Matchup(){}
        Matchup(Team* home_team, Team* away_team, uint day_of_year);

        Game_Result play() {
            home_team->prepare_for_game(day_of_year, true);
            away_team->prepare_for_game(day_of_year, true);
            Game_Result result = Baseball_Game(home_team, away_team, day_of_year).play_game();

            times_played++;
            games_won[result.winner]++;
            for (uint i = 0; i < 2; i++) runs_scored[i] += result.final_score[i];

            for (Player* player : result.home_team->pitchers_used) {
                player->day_of_last_game_played = day_of_year;
            }
            for (Player* player : result.away_team->pitchers_used) {
                player->day_of_last_game_played = day_of_year;
            }

            return result;
        }

        void print_results();
};


class Season {
    public:
        uint year;
        std::vector<Matchup> matchups;
        std::vector<Team*> teams;

        Season(){}
        Season(const std::vector<Team*>& teams, uint year);

        std::vector<Team*> run_games(uint sims_per_matchup);

    private:
        void populate_matchups();
        eTeam simulate_matchup(Matchup& matchup);
};

/* What data do I want to have on the series?
For each game:
    - Winrate
    - Runs Scored
This will do for now
*/
class Series {
    public:
        uint total_games_played = 0;

        Series(Team* home_team, Team* away_team, uint games_in_series, uint num_simulations);
        eTeam play();
        void print_results();

    private:
        std::vector<Matchup> matchups;
        Team* teams[2];
        uint series_won[2]{0};   // Keeps track of how many times each team has won the series
        uint games_played_in_series_won[2]{0}; // Accumulates how many games were played in series where each team won
        uint games_in_series; // Number of games in the series (Ex: For a world series, this would be 7)
        uint num_simulations; // Number of times to simulate the series
        uint games_to_clinch; // Number of games needed to clinch the series

        void populate_matchups();
        Matchup get_series_matchup(uint current_matchup_index);
        eTeam play_series_once();
};