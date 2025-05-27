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

        Matchup(){}
        Matchup(Team* home_team, Team* away_team, uint day_of_year);

        inline Baseball_Game load_game() const {
            return Baseball_Game(home_team, away_team, day_of_year);
        }
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
        eTeam simulate_matchup(const Matchup& matchup);
};


class Series {
    public:
        uint num_games;
        
};