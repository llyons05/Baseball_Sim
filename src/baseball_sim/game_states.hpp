#pragma once

#include "player.hpp"
#include "team.hpp"

#include <stdint.h>


// We will put more in here later (ex: double plays, pop flys, etc.)
struct Ball_In_Play_Result {
    uint8_t batter_bases_advanced = 0;
};


class At_Bat {
    public:
        uint8_t balls;
        uint8_t strikes;

        Team* batting_team;
        Team* pitching_team;

        Player* pitcher;
        Player* batter;

        At_Bat(Team* batting_team, Team* pitching_team);
        eAt_Bat_Outcomes play();
};


class Base_State {
    public:
        Base_State() {}
        Base_State(Team* batting_team, Team* pitching_team) : players_on_base(), batting_team(batting_team), pitching_team(pitching_team) {}

        uint8_t handle_walk(Player* batter);
        uint8_t handle_ball_in_play(Player* batter, const Ball_In_Play_Result& ball_in_play_result);
        uint8_t check_stolen_bases(Player* pitcher);
        bool bases_empty();
        void print();

    private:
        Player* players_on_base[3];
        Team* batting_team;
        Team* pitching_team;

        bool can_simulate_steal(Player* runner, Player* pitcher);
        bool will_runner_attempt_steal(eBases runner_base, Player* pitcher);
        bool will_steal_succeed(eBases runner_starting_base, Player* pitcher);
        uint8_t get_runner_advancement(eBases starting_base, uint8_t batter_bases_advanced, int max_base);

        bool base_occupied(eBases base) {
            return players_on_base[base] != NULL;
        }
};


class Half_Inning {
    public:
        const static uint8_t NUM_OUTS_TO_END_INNING = 3;
        
        Half_Inning(Team* batting_team, Team* pitching_team, uint8_t half_inning_number, uint day_of_year, float runs_to_end_game);
        uint8_t play();
    
    private:
        Team* batting_team;
        Team* pitching_team;

        uint8_t outs;
        uint8_t runs_scored;
        float runs_to_end_game; // This is a float so it can be infinity

        Base_State bases;

        uint8_t half_inning_number;
        uint day_of_year;

        void play_at_bat();
        Ball_In_Play_Result get_ball_in_play_result(Player* batter, Player* pitcher);
        uint8_t get_batter_bases_advanced(Player* batter, Player* pitcher);
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

            home_team->running_stats.runs_scored += final_score[HOME_TEAM];
            home_team->running_stats.runs_allowed += final_score[AWAY_TEAM];
            away_team->running_stats.runs_scored += final_score[AWAY_TEAM];
            away_team->running_stats.runs_allowed += final_score[HOME_TEAM];
        }
};