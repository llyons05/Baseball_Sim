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