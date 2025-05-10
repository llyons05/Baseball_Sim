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


enum eStrike_Types {
    STRIKE_LOOKING,
    STRIKE_SWINGING,
    STRIKE_FOUL,
    STRIKE_IN_PLAY,
    NUM_STRIKE_TYPES
};


enum ePitch_Outcomes {
    PITCH_STRIKE,
    PITCH_BALL,
    PITCH_FOUL,
    PITCH_IN_PLAY,
    NUM_PITCH_OUTCOMES
};


enum eAt_Bat_Outcomes {
    OUTCOME_BALL_IN_PLAY,
    OUTCOME_WALK,
    OUTCOME_STRIKEOUT,
    NUM_AB_OUTCOMES
};


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

    private:
        // Probability of a pitch being a ball or not (index 0 is strike, 1 is ball)
        float strike_or_ball_probs[2];
        float strike_type_probs[NUM_STRIKE_TYPES];

        void populate_pitch_probabilities();
        ePitch_Outcomes get_pitch_outcome();
        eAt_Bat_Outcomes get_basic_at_bat_outcome();
        bool should_use_basic_stats();
};


class Base_State {
    public:
        Base_State() {}
        Base_State(Team* batting_team, Team* pitching_team) : players_on_base(), batting_team(batting_team), pitching_team(pitching_team) {}

        uint8_t handle_walk(Player* batter);
        uint8_t handle_ball_in_play(Player* batter, const Ball_In_Play_Result& ball_in_play_result);
        void print();

    private:
        Player* players_on_base[3];
        Team* batting_team;
        Team* pitching_team;

        uint8_t get_runner_advancement(eBases starting_base, uint8_t batter_bases_advanced, int max_base);
};


class Half_Inning {
    public:
        const static uint8_t NUM_OUTS_TO_END_INNING = 3;
        
        Half_Inning(Team* batting_team, Team* pitching_team, uint8_t half_inning_number, unsigned int day_of_year);
        uint8_t play();
    
    private:
        Team* batting_team;
        Team* pitching_team;

        uint8_t outs;
        uint8_t runs_scored;

        Base_State bases;

        uint8_t half_inning_number;
        unsigned int day_of_year;

        void handle_at_bat_outcome(eAt_Bat_Outcomes at_bat_outcome);
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

            home_team->runs_scored += final_score[HOME_TEAM];
            home_team->runs_allowed += final_score[AWAY_TEAM];
            away_team->runs_scored += final_score[AWAY_TEAM];
            away_team->runs_allowed += final_score[HOME_TEAM];
        }
};