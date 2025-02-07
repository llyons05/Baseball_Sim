#pragma once

#include "game_states.hpp"

#include "statistics.hpp"

#include <random>
#include <time.h>

//https://sabr.org/journal/article/matchup-probabilities-in-major-league-baseball/


At_Bat::At_Bat(Team& hitting_team, Team& pitching_team) {
    this->pitcher = &pitching_team.fielders[POS_PITCHER];
    this->batter = &hitting_team.batting_order[hitting_team.position_in_batting_order];
}


eAt_Bat_Result At_Bat::play() {
    return get_ab_result();
}


eAt_Bat_Result At_Bat::get_ab_result() {

    float outcome_probs[num_outcomes];
    calculate_probabilities(outcome_probs);

    float r = ((float)rand())/(RAND_MAX);

    // std::cout << "HIT PROB: " << outcome_probs[0] << "\n";
    // std::cout << "WALK PROB: " << outcome_probs[1] << "\n";
    // std::cout << "OUT PROB: " << outcome_probs[2] << "\n";

    if (r<= outcome_probs[0]) {
        std::cout << "\t" << batter->name << " GOT A " << "HIT!\n";
        return ADVANCED_ONE_BASE;
    }
    r -= outcome_probs[0];
    if (r<= outcome_probs[1]) {
        std::cout << "\t" << batter->name << " WAS " << "WALKED...\n";
        return ADVANCED_ONE_BASE;
    }
    std::cout << "\t" << batter->name << " IS " << "OUT!\n";
    
    return OUT;
}


void At_Bat::calculate_probabilities(float prob_array[num_outcomes]) {
    float plate_appearance_num = 1/get_probability_numerator("b_pa", "p_bfp", "PA", LEAGUE_BATTING);

    // Hit Probability Constant
    float hit_prob_constant = get_probability_numerator("b_h", "p_h", "H", LEAGUE_BATTING) * plate_appearance_num;

    // Walk Probability Constant
    float batter_total_walks = batter->stats.get_stat<float>(PLAYER_BATTING, "b_bb", 0.0) + batter->stats.get_stat<float>(PLAYER_BATTING, "b_ibb", 0.0) + batter->stats.get_stat<float>(PLAYER_BATTING, "b_hbp", 0.0);
    float pitcher_total_walks = pitcher->stats.get_stat<float>(PLAYER_PITCHING, "p_bb", 0.0) + pitcher->stats.get_stat<float>(PLAYER_PITCHING, "p_ibb", 0.0) + pitcher->stats.get_stat<float>(PLAYER_PITCHING, "p_hbp", 0.0);
    float league_total_walks = LEAGUE_AVG_STATS.get_stat<float>(LEAGUE_BATTING, batter->stats.current_year, "BB", 0.0) + LEAGUE_AVG_STATS.get_stat<float>(LEAGUE_BATTING, batter->stats.current_year, "IBB", 0.0) + LEAGUE_AVG_STATS.get_stat<float>(LEAGUE_BATTING, batter->stats.current_year, "HBP", 0.0);
    float walk_prob_constant = (batter_total_walks*pitcher_total_walks/league_total_walks) * plate_appearance_num;

    // Out Probability Constant
    float batter_total_outs = batter->stats.get_stat<float>(PLAYER_BATTING, "b_pa", 0.0) - (batter->stats.get_stat<float>(PLAYER_BATTING, "b_h", 0.0) + batter_total_walks);
    float pitcher_total_outs = pitcher->stats.get_stat<float>(PLAYER_PITCHING, "p_bfp", 0.0) - (pitcher->stats.get_stat<float>(PLAYER_PITCHING, "p_h", 0.0) + pitcher_total_walks);
    float league_total_outs = LEAGUE_AVG_STATS.get_stat<float>(LEAGUE_BATTING, batter->stats.current_year, "PA", 0.0) - (LEAGUE_AVG_STATS.get_stat<float>(LEAGUE_BATTING, batter->stats.current_year, "H", 0.0) + league_total_walks);
    float out_prob_constant = (batter_total_outs*pitcher_total_outs/league_total_outs) * plate_appearance_num;

    float total_probability_constant = hit_prob_constant + walk_prob_constant + out_prob_constant;

    float hit_probability = hit_prob_constant / total_probability_constant;
    float walk_probability = walk_prob_constant / total_probability_constant;
    float out_probability = out_prob_constant / total_probability_constant;

    float result[num_outcomes] {hit_probability, walk_probability, out_probability};
    
    for (int i = 0; i < num_outcomes; i++) {
        prob_array[i] = result[i];
    }
}


float At_Bat::get_probability_numerator(std::string batter_stat, std::string pitcher_stat, std::string league_stat, eLeague_Stat_Types league_stat_type) {
    float x = batter->stats.get_stat<float>(PLAYER_BATTING, batter_stat, 0.0);
    float y = pitcher->stats.get_stat<float>(PLAYER_PITCHING, pitcher_stat, 0.0);
    float z = LEAGUE_AVG_STATS.get_stat<float>(league_stat_type, batter->stats.current_year, league_stat, 0.0);

    return x*y/z;
}


Half_Inning::Half_Inning(Team& hitting_team, Team& pitching_team) {
    this->hitting_team = &hitting_team;
    this->pitching_team = &pitching_team;
    outs = 0;
    runs_scored = 0;
}


int Half_Inning::play() {
    std::cout << "TEAM AT BAT: " << hitting_team->team_name << "\n";
    while (outs < Half_Inning::NUM_OUTS_TO_END_INNING) {
        At_Bat at_bat(*hitting_team, *pitching_team);
        eAt_Bat_Result at_bat_result = at_bat.play();
        handle_at_bat_result(at_bat_result);
    }
    std::cout << "HALF INNING OVER: RUNS SCORED: " << runs_scored << "\n\n";
    return runs_scored;
}


void Half_Inning::handle_at_bat_result(eAt_Bat_Result at_bat_result) {
    if (at_bat_result == OUT) {
        outs++;
    }
    runs_scored += bases.advance_runners(hitting_team->batting_order[hitting_team->position_in_batting_order], at_bat_result);
    hitting_team->position_in_batting_order = (hitting_team->position_in_batting_order + 1) % 9;
}


int Base_State::advance_runners(Player& batter, eAt_Bat_Result result) {
    if (result == OUT) return 0;

    int runs_scored = 0;
    for (int i = THIRD_BASE; i >= FIRST_BASE; i--) {
        if (players_on_base[i].name != "NULL") {
            int new_base = i + result;
            if (new_base > THIRD_BASE) {
                players_on_base[i] = NULL_PLAYER;
                runs_scored++;
            }
            else {
                players_on_base[new_base] = players_on_base[i];
            }
        }
    }
    int batter_base = result - 1;
    if (batter_base > THIRD_BASE) {
        runs_scored++;
    }
    else {
        players_on_base[batter_base] = batter;
    }
    return runs_scored;
}