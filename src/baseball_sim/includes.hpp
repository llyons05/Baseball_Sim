#pragma once

#include <variant>
#include <string>

#if BASEBALL_DEBUG
    #define debug_print(debug_string) std::cout << __FILE__ << ":" << __LINE__ << "    " << debug_string << std::endl;
    #define debug_line(line_of_code) line_of_code;
#else
    #define debug_print(debug_string) /*Print a string if we are in debug mode*/
    #define debug_line(line_of_code) /*Compile a line of code only if we are in debug mode*/
#endif


#if BASEBALL_VIEW
    #define game_viewer_print(print_string) std::cout << print_string;
    #define game_viewer_line(line_of_code) line_of_code;
#else
    #define game_viewer_print(print_string) /*Print a string if we are in game viewing mode*/
    #define game_viewer_line(line_of_code) /*Compile a line of code only if we are in game viewing mode*/
#endif


enum eTeam {
    AWAY_TEAM,
    HOME_TEAM
};

enum eBases {
    FIRST_BASE,
    SECOND_BASE,
    THIRD_BASE,
    HOME_PLATE
};

enum eAt_Bat_Outcomes {
    OUTCOME_BALL_IN_PLAY,
    OUTCOME_WALK,
    OUTCOME_STRIKEOUT,
    NUM_AB_OUTCOMES
};

enum eDefensivePositions {
    POS_PITCHER,
    POS_CATCHER,
    POS_1B,
    POS_2B,
    POS_3B,
    POS_SS,
    POS_LF,
    POS_CF,
    POS_RF,
    POS_DH,
    NUM_DEFENSIVE_POSITIONS,
    POS_NONE
};

const std::string DEFENSIVE_POSITIONS[NUM_DEFENSIVE_POSITIONS] = {"pitcher", "catcher", "1B", "2B", "3B", "SS", "LF", "CF", "RF", "DH"};

const std::string BASERUNNING_STAT_STRINGS[2][2][3] = {{{"on_first_single", "on_first_single_13"}, 
                                                        {"on_first_double", "on_first_double_1H"}},
                                                       {{"on_second_single", "on_second_single_2H"}}};
// [BASE][STOLEN/CAUGHT]
const std::string BASE_STEALING_STAT_STRINGS[2][2] = {{"SB_2", "CS_2"},
                                                      {"SB_3", "CS_3"}};

// Translates from base to player who is defending that base
const eDefensivePositions BASE_TO_POSITION_KEY[4] = {POS_1B, POS_2B, POS_3B, POS_CATCHER};