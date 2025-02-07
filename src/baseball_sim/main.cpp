#include "load_stats.hpp"
#include "baseball_game.hpp"

#include <iostream>


int main() {
    Stat_Loader loader;
    loader.load_league_avgs();

    Team home_team = loader.load_team("ATL", 2024);
    Team away_team = loader.load_team("BAL", 2024);

    home_team.print_fielders();
    away_team.print_fielders();

    home_team.print_batting_order();
    away_team.print_batting_order();

    Baseball_Game game(home_team, away_team);
    Game_Result result = game.play_game();
}