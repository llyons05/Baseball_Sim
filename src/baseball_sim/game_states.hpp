#pragma once

#include "player.hpp"
#include "team.hpp"

enum eBases {
    FIRST_BASE,
    SECOND_BASE,
    THIRD_BASE,
    HOME_PLATE
};


enum eAt_Bat_Result {
    BATTER_OUT,
    SINGLE,
    DOUBLE,
    TRIPLE,
    HOME_RUN,
    BATTER_WALKED,
    NUM_AT_BAT_RESULTS
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
        eAt_Bat_Result play();
        eAt_Bat_Result get_ab_result();

    private:
        const static int num_true_outcomes = 3;

        int get_true_outcome();
        eAt_Bat_Result get_hit_result();
};


class Base_State {
    public:
        Player* players_on_base[3];

        Team* batting_team;
        Team* pitching_team;

        Base_State() {}
        Base_State(Team* batting_team, Team* pitching_team) : players_on_base(), batting_team(batting_team), pitching_team(pitching_team) {}

        int advance_runners(Player* batter, eAt_Bat_Result result);
        int handle_walk(Player* batter);
        int handle_hit(Player* batter, eAt_Bat_Result result);
        int get_player_advancement(eBases starting_base, eAt_Bat_Result batter_bases_advanced, int max_base);

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

        Half_Inning(Team* batting_team, Team* pitching_team, int half_inning_number);
        int play();
        void handle_at_bat_result(eAt_Bat_Result at_bat_result);
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