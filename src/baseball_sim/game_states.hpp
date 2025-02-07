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
    OUT,
    ADVANCED_ONE_BASE,
    ADVANCED_TWO_BASES,
    ADVANCED_THREE_BASES,
    HOME_RUN
};


class At_Bat {
    public:
        const static int NUM_BALLS_TO_WALK = 4;
        const static int NUM_STRIKES_TO_OUT = 3;

        int balls;
        int strikes;

        Player* pitcher;
        Player* batter;

        At_Bat(Team& hitting_team, Team& pitching_team);
        eAt_Bat_Result play();
        eAt_Bat_Result get_ab_result();

    private:
        const static int num_outcomes = 3;

        void calculate_probabilities(float prob_array[num_outcomes]);
        float get_probability_numerator(std::string batter_stat, std::string pitcher_stat, std::string league_stat, eLeague_Stat_Types league_stat_type);
        int get_random_event(float event_probs[], int num_events);
        eAt_Bat_Result get_hit_result();
};


class Base_State {
    public:
        Player players_on_base[3];

        Base_State() {
            for (int i = 0; i < 3; i++) {
                players_on_base[i] = NULL_PLAYER;
            }
        }

        int advance_runners(Player& batter, eAt_Bat_Result result);
};


class Half_Inning {
    public:
        const static int NUM_OUTS_TO_END_INNING = 3;

        Team* hitting_team;
        Team* pitching_team;

        Player* pitcher;
        Player* batter;

        int outs;
        int runs_scored;
        Base_State bases;

        Half_Inning(Team& hitting_team, Team& pitching_team);
        int play();
        void handle_at_bat_result(eAt_Bat_Result at_bat_result);
};


class Game_Result {
    public:
        Team teams[2];
        int final_score[2];
        int half_innings_played;

        Game_Result(Team home_team, Team away_team, int final_score[2], int half_innings_played) {
            this->teams[HOME_TEAM] = home_team;
            this->teams[AWAY_TEAM] = away_team;
            this->final_score[0] = final_score[0];
            this->final_score[1] = final_score[1];
            this->half_innings_played = half_innings_played;
        }
};