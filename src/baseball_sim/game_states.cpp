#include "game_states.hpp"

#include "includes.hpp"
#include "probability.hpp"
#include "statistics.hpp"
#include "user_interface.hpp"

#include <random>
#include <time.h>
#include <cassert>
#include <string>

//https://sabr.org/journal/article/matchup-probabilities-in-major-league-baseball/

const float MIN_DEFENSE_STOLEN_BASE_PROB = .01;
const float MAX_DEFENSE_STOLEN_BASE_PROB = .99;


At_Bat::At_Bat(Team* batting_team, Team* pitching_team) {
    this->pitcher = pitching_team->fielders[POS_PITCHER];
    this->batter = batting_team->get_batter();
    this->batting_team = batting_team;
    this->pitching_team = pitching_team;
    strikes = 0;
    balls = 0;
}


eAt_Bat_Outcomes At_Bat::play() {
    game_viewer_print("\n\tUp to bat: " + batter->name + "\n");
    float batter_probs[NUM_AB_OUTCOMES];
    float pitcher_probs[NUM_AB_OUTCOMES];
    const float* league_probs = ALL_LEAGUE_STATS[batter->stats.current_year].at_bat_probs;

    int batter_plate_appearances = batter->stats.get_stat(PLAYER_BATTING, "b_pa", .0f);
    if (batter_plate_appearances == 0) {
        debug_print(batter->name + " has no batting plate appearances, defaulting to 0...\n");
        batter_probs[OUTCOME_STRIKEOUT] = 1;
        batter_probs[OUTCOME_WALK] = 0;
    }
    else {
        batter_probs[OUTCOME_STRIKEOUT] = batter->stats.get_stat(PLAYER_BATTING, "b_so", .0f)/batter_plate_appearances;
        batter_probs[OUTCOME_WALK] = (batter->stats.get_stat(PLAYER_BATTING, "b_bb", .0f) + batter->stats.get_stat(PLAYER_BATTING, "b_hbp", .0f))/batter_plate_appearances;
    }
    batter_probs[OUTCOME_BALL_IN_PLAY] = 1 - batter_probs[OUTCOME_STRIKEOUT] - batter_probs[OUTCOME_WALK];

    int pitcher_plate_appearances = pitcher->stats.get_stat(PLAYER_PITCHING, "p_bfp", .0f);
    if (pitcher_plate_appearances == 0) {
        debug_print(pitcher->name + " has no pitching plate appearances, defaulting to league avg...\n");
        pitcher_probs[OUTCOME_STRIKEOUT] = league_probs[OUTCOME_STRIKEOUT];
        pitcher_probs[OUTCOME_WALK] = league_probs[OUTCOME_WALK];
    }
    else {
        pitcher_probs[OUTCOME_STRIKEOUT] = pitcher->stats.get_stat(PLAYER_PITCHING, "p_so", .0f)/pitcher_plate_appearances;
        pitcher_probs[OUTCOME_WALK] = (pitcher->stats.get_stat(PLAYER_PITCHING, "p_bb", .0f) + pitcher->stats.get_stat(PLAYER_PITCHING, "p_hbp", .0f))/pitcher_plate_appearances;
    }
    pitcher_probs[OUTCOME_BALL_IN_PLAY] = 1 - pitcher_probs[OUTCOME_STRIKEOUT] - pitcher_probs[OUTCOME_WALK];

    float outcome_probs[NUM_AB_OUTCOMES];
    calculate_event_probabilities(batter_probs, pitcher_probs, league_probs, outcome_probs, NUM_AB_OUTCOMES);
    return (eAt_Bat_Outcomes) get_random_event(outcome_probs, NUM_AB_OUTCOMES);
}


Half_Inning::Half_Inning(Team* batting_team, Team* pitching_team, uint8_t half_inning_number, uint day_of_year) {
    this->batting_team = batting_team;
    this->pitching_team = pitching_team;
    this->half_inning_number = half_inning_number;
    this->day_of_year = day_of_year;
    this->bases = Base_State(batting_team, pitching_team);
    outs = 0;
    runs_scored = 0;
}


uint8_t Half_Inning::play() {
    game_viewer_line(
        std::string top_or_bottom = "TOP ";
        if (half_inning_number % 2) top_or_bottom = "BOTTOM ";
        std::cout << top_or_bottom << half_inning_number / 2 + 1<< "\n";
        std::cout << "TEAM AT BAT: " << batting_team->team_name << "\n";
    );

    while (outs < Half_Inning::NUM_OUTS_TO_END_INNING) {
        play_at_bat();
        game_viewer_line(bases.print());
        game_viewer_line(wait_for_user_input(""));
    }

    game_viewer_print("HALF INNING OVER: RUNS SCORED: " << (int)runs_scored << "\n\n");
    return runs_scored;
}

// Check pitcher switch calling, it should be here
void Half_Inning::play_at_bat() {
    pitching_team->try_switching_pitcher(half_inning_number, day_of_year);
    outs += bases.check_stolen_bases(pitching_team->get_pitcher());

    if (outs < 3) {
        global_stats.total_PAs++;
        uint8_t runs_from_at_bat = 0;
        At_Bat at_bat(batting_team, pitching_team);
        eAt_Bat_Outcomes at_bat_outcome = at_bat.play();

        if (at_bat_outcome == OUTCOME_STRIKEOUT) {
            game_viewer_print("\tSTRIKEOUT!\n");
            outs++;
        }
        else if (at_bat_outcome == OUTCOME_WALK) {
            game_viewer_print("\tWALK...\n");
            runs_from_at_bat = bases.handle_walk(batting_team->get_batter());
        }
        else { // Ball in play
            global_stats.balls_in_play++;
            Ball_In_Play_Result result = get_ball_in_play_result(batting_team->get_batter(), pitching_team->get_pitcher());
            runs_from_at_bat = bases.handle_ball_in_play(batting_team->get_batter(), result);

            if (result.batter_bases_advanced == 0) {
                game_viewer_print("\tBATTER WAS PUT OUT!\n");
                outs++;
            }
        }
        runs_scored += runs_from_at_bat;
        batting_team->position_in_batting_order = (batting_team->position_in_batting_order + 1) % 9;
        pitching_team->runs_allowed_by_pitcher += runs_from_at_bat;
    }
}


Ball_In_Play_Result Half_Inning::get_ball_in_play_result(Player* batter, Player* pitcher) {
    Ball_In_Play_Result result;

    // Probabilities of getting a hit or getting out, index 0 is hit, index 1 is out.
    float batter_hit_probs[2];
    float pitcher_hit_probs[2];
    const float* league_hit_probs = ALL_LEAGUE_STATS[batter->stats.current_year].hit_or_out_probs;

    float batter_balls_in_play = batter->stats.get_stat(PLAYER_BATTING, "b_pa", .0f) - batter->stats.get_stat(PLAYER_BATTING, "b_bb", .0f)
                               - batter->stats.get_stat(PLAYER_BATTING, "b_hbp", .0f) - batter->stats.get_stat(PLAYER_BATTING, "b_so", .0f);
    float batter_hits = batter->stats.get_stat(PLAYER_BATTING, "b_h", .0f);
    if (batter_balls_in_play == 0)
        batter_hit_probs[0] = 0; // If we have no data for batter, we just assume that the batter is always out so that batters with no baserunning file never get on base
    else
        batter_hit_probs[0] = batter_hits/batter_balls_in_play;
    batter_hit_probs[1] = 1 - batter_hit_probs[0];

    int pitcher_balls_in_play = pitcher->stats.get_stat(PLAYER_PITCHING, "p_bfp", .0f) - pitcher->stats.get_stat(PLAYER_PITCHING, "p_bb", .0f)
                              - pitcher->stats.get_stat(PLAYER_PITCHING, "p_hbp", .0f) - pitcher->stats.get_stat(PLAYER_PITCHING, "p_so", .0f);
    float pitcher_hits = pitcher->stats.get_stat(PLAYER_PITCHING, "p_h", .0f);
    if ((pitcher_balls_in_play == 0) || (pitcher_hits == 0))
        pitcher_hit_probs[0] = league_hit_probs[0]; // If we have no data for pitcher, we just give them the league avg stats
    else
        pitcher_hit_probs[0] = pitcher_hits/pitcher_balls_in_play;
    pitcher_hit_probs[1] = 1 - pitcher_hit_probs[0];

    float hit_probs[2];
    calculate_event_probabilities(batter_hit_probs, pitcher_hit_probs, league_hit_probs, hit_probs, 2);
    uint8_t hit_or_out = get_random_event(hit_probs, 2);

    if (hit_or_out == 1) // if batter is out
        result.batter_bases_advanced = 0;
    else // if batter got a hit
        result.batter_bases_advanced = get_batter_bases_advanced(batter, pitcher);
    
    return result;
}


uint8_t Half_Inning::get_batter_bases_advanced(Player* batter, Player* pitcher) {
    global_stats.total_hits++;
    float batter_probs[4];
    float outcome_probabilities[4];

    const int batter_total_hits = batter->stats.get_stat(PLAYER_BATTING, "b_h", 1.f);
    batter_probs[1] = batter->stats.get_stat(PLAYER_BATTING, "b_doubles", .0f)/batter_total_hits;
    batter_probs[2] = batter->stats.get_stat(PLAYER_BATTING, "b_triples", .0f)/batter_total_hits;
    batter_probs[3] = batter->stats.get_stat(PLAYER_BATTING, "b_hr", .0f)/batter_total_hits;
    batter_probs[0] = 1 - batter_probs[1] - batter_probs[2] - batter_probs[3];

    if (!pitcher->stats[PLAYER_BATTING_AGAINST].empty()) {
        const float* league_probs = ALL_LEAGUE_STATS[batter->stats.current_year].hit_type_probs;
        float pitcher_probs[4];

        const int pitcher_total_hits = pitcher->stats.get_stat(PLAYER_BATTING_AGAINST, "H", .0f);
        if (pitcher_total_hits == 0)
            for (int i = 0; i < 4; i++)
                pitcher_probs[i] = league_probs[i];
        else {
            pitcher_probs[1] = pitcher->stats.get_stat(PLAYER_BATTING_AGAINST, "2B", .0f)/pitcher_total_hits;
            pitcher_probs[2] = pitcher->stats.get_stat(PLAYER_BATTING_AGAINST, "3B", .0f)/pitcher_total_hits;
            pitcher_probs[3] = pitcher->stats.get_stat(PLAYER_BATTING_AGAINST, "HR", .0f)/pitcher_total_hits;
            pitcher_probs[0] = 1 - pitcher_probs[1] - pitcher_probs[2] - pitcher_probs[3];
        }
        
        calculate_event_probabilities(batter_probs, pitcher_probs, league_probs, outcome_probabilities, 4);
    }
    else {
        debug_line(game_viewer_print("No batting_against file available for " + pitcher->name + ", using batter probs only for batter bases advanced...\n"));
        for (int i = 0; i < 4; i++)
            outcome_probabilities[i] = batter_probs[i];
    }
    uint8_t bases_advanced = get_random_event(outcome_probabilities, 4) + 1;

    game_viewer_line(
        if (bases_advanced == 1) std::cout << "\tSINGLE\n";
        else if (bases_advanced == 2) std::cout << "\tDOUBLE\n";
        else if (bases_advanced == 3) std::cout << "\tTRIPLE\n";
        else if (bases_advanced == 4) std::cout << "\tHOME RUN!!!\n";
    )
    debug_line(
        if (batter_total_hits == 0)
            std::cout << "WARNING: batter " + batter->name + " got a hit against pitcher " + pitcher->name + " despite having no career hits.\n";
    )

    return bases_advanced;
}


bool Base_State::bases_empty() {
    return (players_on_base[FIRST_BASE] == players_on_base[SECOND_BASE]) && (players_on_base[FIRST_BASE] == players_on_base[THIRD_BASE]);
}


bool Base_State::can_simulate_steal(Player* runner, Player* pitcher) {
    return runner->stats[PLAYER_BASERUNNING].size() && pitcher->stats[PLAYER_BATTING_AGAINST].size();
}


// Checks to see if any of the baserunners (if there are any) tried to steal, and if so, returns the number of outs (if any) that resulted from the play.
uint8_t Base_State::check_stolen_bases(Player* pitcher) {
    uint8_t outs = 0;
    for (int i = SECOND_BASE; i >= FIRST_BASE; i--) {
        if (!base_occupied((eBases)(i+1)) && base_occupied((eBases)i)) {
            if (can_simulate_steal(players_on_base[i], pitcher) && will_runner_attempt_steal((eBases)i, pitcher)) {
                if (will_steal_succeed((eBases)i, pitcher)) {
                    game_viewer_print(players_on_base[i]->name +" STOLE BASE "<< i+2 << "\n");
                    players_on_base[i+1] = players_on_base[i];
                }
                else {
                    game_viewer_print(players_on_base[i]->name +" WAS CAUGHT STEALING BASE "<< i+2 << "\n");
                    outs++;
                }
                players_on_base[i] = NULL;
                game_viewer_line(print();wait_for_user_input(""));
            }
        }
    }
    return outs;
}


bool Base_State::will_runner_attempt_steal(eBases runner_base, Player* pitcher) {
    debug_line(assert((runner_base == FIRST_BASE) || (runner_base == SECOND_BASE)))

    Player* runner = players_on_base[runner_base];

    // Will the runner steal or not: index 1 == yes, index 0 == no
    float runner_attempt_probs[2];
    float pitcher_attempt_probs[2];
    const float* league_attempt_probs = ALL_LEAGUE_STATS[pitcher->stats.current_year].steal_attempt_probs[runner_base];

    float runner_sbo_multiplier = ALL_LEAGUE_STATS[runner->stats.current_year].sbo_on_first_percent;
    float pitcher_sbo_multiplier = ALL_LEAGUE_STATS[pitcher->stats.current_year].sbo_on_first_percent;
    if (runner_base == SECOND_BASE) {
        runner_sbo_multiplier = 1 - runner_sbo_multiplier;
        pitcher_sbo_multiplier = 1 - pitcher_sbo_multiplier;
    }

    float runner_sbo = runner->stats.get_stat(PLAYER_BASERUNNING, "SB_opp", .0f)*runner_sbo_multiplier;
    if (runner_sbo == 0)
        runner_attempt_probs[1] = 0;
    else
        runner_attempt_probs[1] = (runner->stats.get_stat(PLAYER_BASERUNNING, BASE_STEALING_STAT_STRINGS[runner_base][0], .0f) + runner->stats.get_stat(PLAYER_BASERUNNING, BASE_STEALING_STAT_STRINGS[runner_base][1], .0f))/runner_sbo;;
    runner_attempt_probs[0] = 1 - runner_attempt_probs[1];

    float pitcher_sbo = pitcher->stats.get_stat(PLAYER_BASERUNNING_AGAINST, "SB_opp", .0f)*pitcher_sbo_multiplier;
    if (pitcher_sbo <= 20)
        pitcher_attempt_probs[1] = league_attempt_probs[1];
    else
        pitcher_attempt_probs[1] = (pitcher->stats.get_stat(PLAYER_BASERUNNING_AGAINST, BASE_STEALING_STAT_STRINGS[runner_base][0], .0f) + pitcher->stats.get_stat(PLAYER_BASERUNNING_AGAINST, BASE_STEALING_STAT_STRINGS[runner_base][1], .0f))/pitcher_sbo;
    pitcher_attempt_probs[0] = 1 - pitcher_attempt_probs[1];

    
    float attempt_probs[2];
    calculate_event_probabilities(runner_attempt_probs, pitcher_attempt_probs, league_attempt_probs, attempt_probs, 2);
    return get_random_event(attempt_probs, 2);
}


bool Base_State::will_steal_succeed(eBases runner_starting_base, Player* pitcher) {
    Player* runner = players_on_base[runner_starting_base];
    Player* baseman = pitching_team->fielders[BASE_TO_POSITION_KEY[runner_starting_base+1]];

    // Will the runner successfully steal: index 1 == yes, index 0 == no
    float runner_probs[2];
    float defense_probs[2];
    const float* league_probs = ALL_LEAGUE_STATS[pitcher->stats.current_year].steal_success_probs[runner_starting_base];

    float runner_steals = runner->stats.get_stat(PLAYER_BASERUNNING, BASE_STEALING_STAT_STRINGS[runner_starting_base][0], .0f);
    float runner_caught = runner->stats.get_stat(PLAYER_BASERUNNING, BASE_STEALING_STAT_STRINGS[runner_starting_base][1], .0f);
    if (runner_steals + runner_caught == 0)
        runner_probs[1] = runner->stats.get_stat(PLAYER_BASERUNNING, "stolen_base_perc", .0f)/100;
    else
        runner_probs[1] = runner_steals/(runner_steals + runner_caught);
    runner_probs[0] = 1 - runner_probs[1];

    float pitcher_steals = pitcher->stats.get_stat(PLAYER_BASERUNNING_AGAINST, BASE_STEALING_STAT_STRINGS[runner_starting_base][0], .0f);
    float pitcher_caught = pitcher->stats.get_stat(PLAYER_BASERUNNING_AGAINST, BASE_STEALING_STAT_STRINGS[runner_starting_base][1], .0f);
    float baseman_fielding = baseman->stats.get_stat(PLAYER_FIELDING, "f_fielding_perc", .0f);
    if (pitcher_steals + pitcher_caught == 0)
        defense_probs[1] = league_probs[1];
    else
        defense_probs[1] = std::clamp(baseman_fielding * pitcher_steals/(pitcher_steals + pitcher_caught), MIN_DEFENSE_STOLEN_BASE_PROB, MAX_DEFENSE_STOLEN_BASE_PROB); // We don't want pitchers who never ever give up stolen bases
    defense_probs[0] = 1 - defense_probs[1];

    float success_probs[2];
    calculate_event_probabilities(runner_probs, defense_probs, league_probs, success_probs, 2);
    return get_random_event(success_probs, 2);
}


// Return runs scored after hit
uint8_t Base_State::handle_ball_in_play(Player* batter, const Ball_In_Play_Result& result) {
    if (result.batter_bases_advanced == 0) {
        return 0;
    }

    uint8_t runs_scored = 0;
    int max_base = HOME_PLATE + 1; // Makes sure that runners can't pass other runners
    for (int i = THIRD_BASE; i >= FIRST_BASE; i--) {

        if (base_occupied((eBases)i)) {
            int new_base = i + get_runner_advancement((eBases)i, result.batter_bases_advanced, max_base);
            if (new_base > THIRD_BASE) {
                game_viewer_print("\t -" << players_on_base[i]->name << " SCORED\n");
                runs_scored++;
            }
            else {
                players_on_base[new_base] = players_on_base[i];
                if (new_base < max_base) max_base = new_base;
            }

            players_on_base[i] = NULL;
        }
    }

    int batter_base = result.batter_bases_advanced - 1;
    if (batter_base > THIRD_BASE) {
        game_viewer_print("\t -" << batter->name << " SCORED\n");
        runs_scored++;
    }
    else {
        players_on_base[batter_base] = batter;
    }
    return runs_scored;
}


uint8_t Base_State::get_runner_advancement(eBases starting_base, uint8_t batter_bases_advanced, int max_base) {
    if ((starting_base + batter_bases_advanced > THIRD_BASE) || (starting_base + batter_bases_advanced >= max_base - 1) || (players_on_base[starting_base]->stats[PLAYER_BASERUNNING].empty())) {
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


uint8_t Base_State::handle_walk(Player* batter) {
    uint8_t runs_scored = 0;
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
    const char empty_base = 'o';
    const char full_base = (char)254;
    // Print out second base
    if (base_occupied(SECOND_BASE)) {
        std::cout << "\t\t" << full_base;
    }
    else {
        std::cout << "\t\t" << empty_base;
    }
    // Print out the names of the players on base
    if (base_occupied(FIRST_BASE)) {
        std::cout << "\t\t1st: " << players_on_base[FIRST_BASE]->name;
    }
    std::cout << "\n";
    if (base_occupied(SECOND_BASE)) {
        std::cout << "\t\t\t\t2nd: " << players_on_base[SECOND_BASE]->name;
    }
    std::cout << "\n";

    // Print out first and third base
    if (base_occupied(THIRD_BASE)) {
        std::cout << "\t" << full_base;
    }
    else {
        std::cout << "\t" << empty_base;
    }

    if (base_occupied(FIRST_BASE)) {
        std::cout << "\t\t" << full_base;
    }
    else {
        std::cout << "\t\t" << empty_base;
    }

    if (base_occupied(THIRD_BASE)) {
        std::cout << "\t3rd: " << players_on_base[THIRD_BASE]->name;
    }

    std::cout << "\n\n\t\tH\n";
}