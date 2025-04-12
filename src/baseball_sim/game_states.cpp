#include "game_states.hpp"

#include "probability.hpp"
#include "statistics.hpp"

#include <random>
#include <time.h>
#include <cassert>
#include <string>

//https://sabr.org/journal/article/matchup-probabilities-in-major-league-baseball/


const std::string BASERUNNING_STAT_STRINGS[2][2][3] = {{{"on_first_single", "on_first_single_13"}, 
                                                        {"on_first_double", "on_first_double_1H"}},
                                                       {{"on_second_single", "on_second_single_2H"}}};


At_Bat::At_Bat(Team* batting_team, Team* pitching_team) {
    this->pitcher = pitching_team->fielders[POS_PITCHER];
    this->batter = batting_team->batting_order[batting_team->position_in_batting_order];
    this->batting_team = batting_team;
    this->pitching_team = pitching_team;
}


eAt_Bat_Result At_Bat::play() {
    return get_ab_result();
}


eAt_Bat_Result At_Bat::get_ab_result() {
    const int outcome = get_true_outcome();

    if (outcome == 0) {
        #if BASEBALL_VIEW
        std::cout << "\t" << batter->name << " GOT A HIT!\n";
        #endif
        return get_hit_result();
    }
    if (outcome == 1) {
        #if BASEBALL_VIEW
        std::cout << "\t" << batter->name << " WAS WALKED...\n";
        #endif
        return BATTER_WALKED;
    }

    #if BASEBALL_VIEW
    std::cout << "\t" << batter->name << " IS OUT!\n";
    #endif

    return BATTER_OUT;
}


int At_Bat::get_true_outcome() {
    float batter_probs[num_true_outcomes];
    float pitcher_probs[num_true_outcomes];
    float league_probs[num_true_outcomes];

    int batter_plate_appearances = batter->stats.get_stat(PLAYER_BATTING, "b_pa", 1.f);
    if (batter_plate_appearances == 0) batter_plate_appearances = 1; // If the batter has no PAs, we just assume they aren't going to get a hit (This makes it easier for baserunning/hit prob calculation)

    batter_probs[0] = (batter->stats.get_stat(PLAYER_BATTING, "b_h", .0f))/batter_plate_appearances;
    batter_probs[1] = (batter->stats.get_stat(PLAYER_BATTING, "b_bb", .0f)
                    + batter->stats.get_stat(PLAYER_BATTING, "b_hbp", .0f))/batter_plate_appearances;
    batter_probs[2] = 1 - batter_probs[0] - batter_probs[1];

    const int pitcher_plate_appearances = pitcher->stats.get_stat(PLAYER_PITCHING, "p_bfp", .0f);
    if (pitcher_plate_appearances == 0) {
        for (int i = 0; i < num_true_outcomes; i++)
            pitcher_probs[i] = batter_probs[i];
    }
    else {
        pitcher_probs[0] = (pitcher->stats.get_stat(PLAYER_PITCHING, "p_h", .0f))/pitcher_plate_appearances;
        pitcher_probs[1] = (pitcher->stats.get_stat(PLAYER_PITCHING, "p_bb", .0f)
                        + pitcher->stats.get_stat(PLAYER_PITCHING, "p_hbp", .0f))/pitcher_plate_appearances;
        pitcher_probs[2] = 1 - pitcher_probs[0] - pitcher_probs[1];
    }

    const float league_plate_appearances = LEAGUE_AVG_STATS.get_stat(LEAGUE_BATTING, batter->stats.current_year, "PA", 1.f);
    league_probs[0] = LEAGUE_AVG_STATS.get_stat(LEAGUE_BATTING, batter->stats.current_year, "H", .0f)/league_plate_appearances;
    league_probs[1] = (LEAGUE_AVG_STATS.get_stat(LEAGUE_BATTING, batter->stats.current_year, "BB", .0f)
                    + LEAGUE_AVG_STATS.get_stat(LEAGUE_BATTING, batter->stats.current_year, "HBP", .0f))/league_plate_appearances;
    league_probs[2] = 1 - league_probs[0] - league_probs[1];


    float outcome_probabilities[num_true_outcomes];
    calculate_event_probabilities(batter_probs, pitcher_probs, league_probs, outcome_probabilities, num_true_outcomes);
    return get_random_event(outcome_probabilities, num_true_outcomes);
}


eAt_Bat_Result At_Bat::get_hit_result() {
    float batter_probs[4];
    float pitcher_probs[4];
    float league_probs[4];

    const int batter_total_hits = batter->stats.get_stat(PLAYER_BATTING, "b_h", 1.f);
    batter_probs[1] = batter->stats.get_stat(PLAYER_BATTING, "b_doubles", .0f)/batter_total_hits;
    batter_probs[2] = batter->stats.get_stat(PLAYER_BATTING, "b_triples", .0f)/batter_total_hits;
    batter_probs[3] = batter->stats.get_stat(PLAYER_BATTING, "b_hr", .0f)/batter_total_hits;
    batter_probs[0] = 1 - batter_probs[1] - batter_probs[2] - batter_probs[3];

    const int pitcher_total_hits = pitcher->stats.get_stat(PLAYER_BATTING_AGAINST, "H", .0f);
    if (pitcher_total_hits == 0) {
        for (int i = 0; i < 4; i++)
            pitcher_probs[i] = batter_probs[i];
    }
    else {
        pitcher_probs[1] = pitcher->stats.get_stat(PLAYER_BATTING_AGAINST, "2B", .0f)/pitcher_total_hits;
        pitcher_probs[2] = pitcher->stats.get_stat(PLAYER_BATTING_AGAINST, "3B", .0f)/pitcher_total_hits;
        pitcher_probs[3] = pitcher->stats.get_stat(PLAYER_BATTING_AGAINST, "HR", .0f)/pitcher_total_hits;
        pitcher_probs[0] = 1 - pitcher_probs[1] - pitcher_probs[2] - pitcher_probs[3];
    }

    const int league_total_hits = LEAGUE_AVG_STATS.get_stat(LEAGUE_BATTING, batter->stats.current_year, "H", 1.f);
    league_probs[1] = LEAGUE_AVG_STATS.get_stat(LEAGUE_BATTING, batter->stats.current_year, "2B", .0f)/league_total_hits;
    league_probs[2] = LEAGUE_AVG_STATS.get_stat(LEAGUE_BATTING, batter->stats.current_year, "3B", .0f)/league_total_hits;
    league_probs[3] = LEAGUE_AVG_STATS.get_stat(LEAGUE_BATTING, batter->stats.current_year, "HR", .0f)/league_total_hits;
    league_probs[0] = 1 - league_probs[1] - league_probs[2] - league_probs[3];
    
    float outcome_probabilities[4];
    calculate_event_probabilities(batter_probs, pitcher_probs, league_probs, outcome_probabilities, 4);
    eAt_Bat_Result result = (eAt_Bat_Result)(get_random_event(outcome_probabilities, 4) + 1);

    #if BASEBALL_VIEW
    if (result == SINGLE) std::cout << "\t SINGLE\n";
    else if (result == DOUBLE) std::cout << "\t DOUBLE\n";
    else if (result == TRIPLE) std::cout << "\t TRIPLE\n";
    else if (result == HOME_RUN) std::cout << "\t HOME RUN!!!\n";
    #endif

    return result;
}


Half_Inning::Half_Inning(Team* batting_team, Team* pitching_team, int half_inning_number) {
    this->batting_team = batting_team;
    this->pitching_team = pitching_team;
    this->half_inning_number = half_inning_number;
    this->bases = Base_State(batting_team, pitching_team);
    outs = 0;
    runs_scored = 0;
}


int Half_Inning::play() {
    #if BASEBALL_VIEW
    std::string top_or_bottom = "TOP ";
    if (half_inning_number % 2) top_or_bottom = "BOTTOM ";
    std::cout << "\n" << top_or_bottom << half_inning_number / 2 + 1<< "\n";
    std::cout << "TEAM AT BAT: " << batting_team->team_name << "\n";
    #endif

    while (outs < Half_Inning::NUM_OUTS_TO_END_INNING) {
        At_Bat at_bat(batting_team, pitching_team);
        eAt_Bat_Result at_bat_result = at_bat.play();
        handle_at_bat_result(at_bat_result);
        bases.print();
    }

    #if BASEBALL_VIEW
    std::cout << "HALF INNING OVER: RUNS SCORED: " << runs_scored << "\n";
    #endif

    return runs_scored;
}


void Half_Inning::handle_at_bat_result(eAt_Bat_Result at_bat_result) {
    if (at_bat_result == BATTER_OUT) {
        outs++;
    }
    int runs_from_ab = bases.advance_runners(batting_team->batting_order[batting_team->position_in_batting_order], at_bat_result);
    runs_scored += runs_from_ab;
    batting_team->position_in_batting_order = (batting_team->position_in_batting_order + 1) % 9;

    pitching_team->runs_allowed_by_pitcher += runs_from_ab;
    pitching_team->try_switching_pitcher(half_inning_number);
}


int Base_State::advance_runners(Player* batter, eAt_Bat_Result result) {
    if (result == BATTER_OUT) return 0;
    if (result == BATTER_WALKED) return handle_walk(batter);
    return handle_hit(batter, result);
}


int Base_State::handle_hit(Player* batter, eAt_Bat_Result result) {
    int runs_scored = 0;
    int max_base = HOME_PLATE + 1; // Makes sure that runners can't pass other runners
    for (int i = THIRD_BASE; i >= FIRST_BASE; i--) {

        if (players_on_base[i] != NULL) {

            int new_base = i + get_player_advancement((eBases)i, result, max_base);

            if (new_base > THIRD_BASE) {
                runs_scored++;
                #if BASEBALL_VIEW
                std::cout << "\t -" << players_on_base[i]->name << " SCORED\n";
                #endif
            }
            else {
                players_on_base[new_base] = players_on_base[i];
                if (new_base < max_base) max_base = new_base;
            }

            players_on_base[i] = NULL;
        }
    }
    int batter_base = result - 1;
    if (batter_base > THIRD_BASE) {
        runs_scored++;
        #if BASEBALL_VIEW
        std::cout << "\t -" << batter->name << " SCORED\n";
        #endif
    }
    else {
        players_on_base[batter_base] = batter;
    }
    return runs_scored;
}


int Base_State::get_player_advancement(eBases starting_base, eAt_Bat_Result batter_bases_advanced, int max_base) {
    if ((starting_base + batter_bases_advanced > THIRD_BASE) || (starting_base + batter_bases_advanced >= max_base - 1)) {
        return batter_bases_advanced;
    }

    const int times_in_situation = players_on_base[starting_base]->stats.get_stat(PLAYER_BASERUNNING, BASERUNNING_STAT_STRINGS[starting_base][batter_bases_advanced-1][0], .0f);
    float extra_base_percentage;
    if (times_in_situation == 0) { 
        extra_base_percentage = players_on_base[starting_base]->stats.get_stat(PLAYER_BASERUNNING, "extra_bases_taken_perc", .0f)/100;
    }
    else {
        extra_base_percentage = players_on_base[starting_base]->stats.get_stat(PLAYER_BASERUNNING, BASERUNNING_STAT_STRINGS[starting_base][batter_bases_advanced-1][1], .0f)/times_in_situation;
    }

    const float normal_base_percentage = 1.0 - extra_base_percentage;
    float outcomes[2] = {normal_base_percentage, extra_base_percentage};

    return get_random_event(outcomes, 2) + batter_bases_advanced;
}


int Base_State::handle_walk(Player* batter) {
    int runs_scored = 0;
    Player* current_player = players_on_base[FIRST_BASE];
    Player* temp;
    uint8_t current_base = FIRST_BASE;

    while (current_player != NULL) {
        if (current_base >= THIRD_BASE) {
            ++runs_scored;
            break;
        }
        temp = players_on_base[++current_base];
        players_on_base[current_base] = current_player;
        current_player = temp;
    }

    players_on_base[FIRST_BASE] = batter;
    return runs_scored;
}


void Base_State::print() {
    #if BASEBALL_VIEW
    const char empty_base = 'o';
    const char full_base = (char)254;

    if (players_on_base[SECOND_BASE] != NULL) {
        std::cout << "\t\t" << full_base << "\n";
    }
    else {
        std::cout << "\t\t" << empty_base << "\n";
    }

    std::cout << "\n";
    if (players_on_base[THIRD_BASE] != NULL) {
        std::cout << "\t" << full_base;
    }
    else {
        std::cout << "\t" << empty_base;
    }

    if (players_on_base[FIRST_BASE] != NULL) {
        std::cout << "\t\t" << full_base;
    }
    else {
        std::cout << "\t\t" << empty_base;
    }

    std::cout << "\n\n\t\tH\n";

    for (int i = 0; i <= THIRD_BASE; i++) {
        if (players_on_base[i] != NULL)
            std::cout << "\t\tPLAYER ON BASE " << i + 1 << ": " << players_on_base[i]->name << "\n";
    }
    #endif
}