#include "game_states.hpp"

#include "statistics.hpp"

#include <random>
#include <time.h>
#include <cassert>

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
    int outcome_index = get_random_event(outcome_probs, num_outcomes);

    if (outcome_index == 0) {
        // std::cout << "\t" << batter->name << " GOT A HIT!\n";
        return get_hit_result();
    }
    if (outcome_index == 1) {
        // std::cout << "\t" << batter->name << " WAS WALKED...\n";
        return ADVANCED_ONE_BASE;
    }

    // std::cout << "\t" << batter->name << " IS OUT!\n";
    return BATTER_OUT;
}


void At_Bat::calculate_probabilities(float prob_array[num_outcomes]) {
    float plate_appearance_num = 1/get_probability_numerator("b_pa", "p_bfp", "PA", LEAGUE_BATTING);

    // Hit Probability Constant
    float hit_prob_constant = get_probability_numerator("b_h", "p_h", "H", LEAGUE_BATTING) * plate_appearance_num;

    // Walk Probability Constant
    float batter_total_walks = batter->stats.get_stat<float>(PLAYER_BATTING, "b_bb", 0.0) + batter->stats.get_stat<float>(PLAYER_BATTING, "b_hbp", 0.0);
    float pitcher_total_walks = pitcher->stats.get_stat<float>(PLAYER_PITCHING, "p_bb", 0.0) + pitcher->stats.get_stat<float>(PLAYER_PITCHING, "p_hbp", 0.0);
    float league_total_walks = LEAGUE_AVG_STATS.get_stat<float>(LEAGUE_BATTING, batter->stats.current_year, "BB", 0.0) + LEAGUE_AVG_STATS.get_stat<float>(LEAGUE_BATTING, batter->stats.current_year, "HBP", 0.0);
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


// Returns the index of the event that happens
int At_Bat::get_random_event(float event_probs[], int num_events) {
    float r = ((float)rand())/(RAND_MAX);
    for (int i = 0; i < num_events; i++) {
        if (r <= event_probs[i]) {
            return i;
        }
        r -= event_probs[i];
    }
    std::cout << "PROBABILITY ERROR\n";
    return -1;
}


eAt_Bat_Result At_Bat::get_hit_result() {
    int total_hits = batter->stats.get_stat<int>(PLAYER_BATTING, "b_h", 0);
    int doubles = batter->stats.get_stat<int>(PLAYER_BATTING, "b_doubles", 0);
    int triples = batter->stats.get_stat<int>(PLAYER_BATTING, "b_triples", 0);
    int home_runs = batter->stats.get_stat<int>(PLAYER_BATTING, "b_hr", 0);
    int singles = total_hits - doubles - triples - home_runs;

    float single_prob = ((float)singles) / total_hits;
    float double_prob = ((float)doubles) / total_hits;
    float triple_prob = ((float)triples) / total_hits;
    float hr_prob = ((float)home_runs) / total_hits;
    
    float prob_arr[4] = {single_prob, double_prob, triple_prob, hr_prob};

    eAt_Bat_Result result = (eAt_Bat_Result)(get_random_event(prob_arr, 4) + 1);

    // if (result == ADVANCED_ONE_BASE) std::cout << "\t SINGLE\n";
    // else if (result == ADVANCED_TWO_BASES) std::cout << "\t DOUBLE\n";
    // else if (result == ADVANCED_THREE_BASES) std::cout << "\t TRIPLE\n";
    // else if (result == HOME_RUN) std::cout << "\t HOME RUN!!!\n";

    return result;
}


Half_Inning::Half_Inning(Team& hitting_team, Team& pitching_team, int half_inning_number) {
    this->hitting_team = &hitting_team;
    this->pitching_team = &pitching_team;
    this->half_inning_number = half_inning_number;
    outs = 0;
    runs_scored = 0;
}


int Half_Inning::play() {
    // std::string top_or_bottom = "TOP ";
    // if (half_inning_number % 2) top_or_bottom = "BOTTOM ";
    // std::cout << "\n" << top_or_bottom << half_inning_number / 2 + 1<< "\n";
    // std::cout << "TEAM AT BAT: " << hitting_team->team_name << "\n";

    while (outs < Half_Inning::NUM_OUTS_TO_END_INNING) {
        At_Bat at_bat(*hitting_team, *pitching_team);
        eAt_Bat_Result at_bat_result = at_bat.play();
        handle_at_bat_result(at_bat_result);
        // bases.print();
    }
    // std::cout << "HALF INNING OVER: RUNS SCORED: " << runs_scored << "\n\n";
    return runs_scored;
}


void Half_Inning::handle_at_bat_result(eAt_Bat_Result at_bat_result) {
    if (at_bat_result == BATTER_OUT) {
        outs++;
    }
    int runs_from_ab = bases.advance_runners(hitting_team->batting_order[hitting_team->position_in_batting_order], at_bat_result);
    runs_scored += runs_from_ab;
    hitting_team->position_in_batting_order = (hitting_team->position_in_batting_order + 1) % 9;

    pitching_team->runs_allowed_by_pitcher += runs_from_ab;
    pitching_team->try_switching_pitcher(half_inning_number);
}


int Base_State::advance_runners(Player& batter, eAt_Bat_Result result) {
    if (result == BATTER_OUT) return 0;

    int runs_scored = 0;
    for (int i = THIRD_BASE; i >= FIRST_BASE; i--) {
        if (players_on_base[i].name != "NULL") {
            int new_base = i + result;
            if (new_base > THIRD_BASE) {
                runs_scored++;
            }
            else {
                players_on_base[new_base] = players_on_base[i];
            }
            players_on_base[i] = NULL_PLAYER;
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


void Base_State::print() {
    const char empty_base = 'o';
    const char full_base = (char)254;

    if (players_on_base[SECOND_BASE].name != "NULL") {
        std::cout << "\t\t" << full_base << "\n";
    }
    else {
        std::cout << "\t\t" << empty_base << "\n";
    }

    std::cout << "\n";
    if (players_on_base[THIRD_BASE].name != "NULL") {
        std::cout << "\t" << full_base;
    }
    else {
        std::cout << "\t" << empty_base;
    }

    if (players_on_base[FIRST_BASE].name != "NULL") {
        std::cout << "\t\t" << full_base;
    }
    else {
        std::cout << "\t\t" << empty_base;
    }

    std::cout << "\n\n\t\tH\n";
}