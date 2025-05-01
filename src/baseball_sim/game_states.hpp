#pragma once

#include "player.hpp"
#include "team.hpp"

#include <stdint.h>

enum eBases {
    FIRST_BASE,
    SECOND_BASE,
    THIRD_BASE,
    HOME_PLATE
};


enum eTrue_Outcomes {
    OUTCOME_CONTACT,
    OUTCOME_WALK,
    OUTCOME_STRIKEOUT,
    NUM_TRUE_OUTCOMES
};


struct At_Bat_Result {
    uint8_t batter_bases_advanced = 0;
    eTrue_Outcomes true_outcome = OUTCOME_STRIKEOUT;
};


class At_Bat {
    public:
        const static int NUM_BALLS_TO_WALK = 4;
        const static int NUM_STRIKES_TO_OUT = 3;

        int balls;
        int strikes;

        Team* batting_team;
        Team* pitching_team;

        Player* pitcher;
        Player* batter;

        At_Bat(Team* batting_team, Team* pitching_team);
        At_Bat_Result play();

    private:
        eTrue_Outcomes get_true_outcome();
        uint8_t get_batter_bases_advanced();
};


class Base_State {
    public:
        Player* players_on_base[3];

        Team* batting_team;
        Team* pitching_team;

        Base_State() {}
        Base_State(Team* batting_team, Team* pitching_team) : players_on_base(), batting_team(batting_team), pitching_team(pitching_team) {}

        int advance_runners(Player* batter, At_Bat_Result result);
        int handle_walk(Player* batter);
        int handle_hit(Player* batter, At_Bat_Result result);
        int get_player_advancement(eBases starting_base, uint8_t batter_bases_advanced, int max_base);

        void print();
};


class Half_Inning {
    public:
        const static int NUM_OUTS_TO_END_INNING = 3;

        Team* batting_team;
        Team* pitching_team;

        int outs;
        int runs_scored;
        Base_State bases;

        int half_inning_number;
        unsigned int day_of_year;

        Half_Inning(Team* batting_team, Team* pitching_team, int half_inning_number, unsigned int day_of_year);
        int play();
        void handle_at_bat_result(At_Bat_Result at_bat_result);
};


class Game_Result {
    public:
        Team* home_team;
        Team* away_team;
        int final_score[2];
        int half_innings_played;
        eTeam winner;

        Game_Result(Team* home_team, Team* away_team, int final_score[2], int half_innings_played) {
            this->home_team = home_team;
            this->away_team = away_team;
            this->final_score[HOME_TEAM] = final_score[HOME_TEAM];
            this->final_score[AWAY_TEAM] = final_score[AWAY_TEAM];
            this->half_innings_played = half_innings_played;
            
            if (final_score[HOME_TEAM] > final_score[AWAY_TEAM]) {
                winner = HOME_TEAM;
            }
            else {
                winner = AWAY_TEAM;
            }
        }
};