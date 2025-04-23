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
        unsigned int day_of_year;

        Matchup(){}
        Matchup(Team* home_team, Team* away_team, unsigned int day_of_year);

        inline Baseball_Game load_game() const {
            return Baseball_Game(home_team, away_team, day_of_year);
        }
};


class Season {
    public:
        unsigned int year;
        std::vector<Matchup> matchups;
        std::vector<Team*> teams;

        Season(){}
        Season(const std::vector<Team*>& teams, unsigned int year);

        std::vector<Team*> run_games(unsigned int sims_per_matchup);

    private:
        void populate_matchups();
        eTeam simulate_matchup(const Matchup& matchup, unsigned int num_simulations);
};