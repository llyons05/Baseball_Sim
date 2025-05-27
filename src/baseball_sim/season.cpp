#include "season.hpp"

#include "includes.hpp"
#include "user_interface.hpp"
#include "utils.hpp"
#include "player.hpp"
#include "team.hpp"
#include "baseball_game.hpp"

#include <vector>
#include <string>
#include <algorithm>

using namespace std;


Season::Season(const vector<Team*>& teams, uint year) {
    this->teams = teams;
    this->year = year;

    populate_matchups();
}


void Season::populate_matchups() {
    vector<Team*> loaded_teams;
    for (Team* team : teams) {
        const Stat_Table& schedule_table = team->team_stats[TEAM_SCHEDULE];
        for (size_t i = 0; i < schedule_table.size(); i++) {
            const std::string away_team_abbr = schedule_table.get_stat<string>("opp_ID", i, "");
            Team* away_team = team_cache.at(get_team_cache_id(away_team_abbr, year)).get();

            if (find(loaded_teams.begin(), loaded_teams.end(), away_team) != loaded_teams.end()) {
                uint day_of_year = get_day_of_year(schedule_table.get_stat<string>("date_game", i, ""), year);
                matchups.push_back(Matchup(team, away_team, day_of_year));
            }
        }
        loaded_teams.push_back(team);
    }

    sort(matchups.begin(), matchups.end(), [](const Matchup& a, const Matchup& b){return a.day_of_year < b.day_of_year;});
}


// Return the teams in order of win %
vector<Team*> Season::run_games(uint num_season_sims) {
    for (uint i = 0; i < num_season_sims; i++){
        for (Matchup& matchup : matchups) {
            eTeam winner = simulate_matchup(matchup);
            if (winner == HOME_TEAM) {
                matchup.home_team->wins++;
                matchup.away_team->losses++;
            }
            else {
                matchup.away_team->wins++;
                matchup.home_team->losses++;
            }

            game_viewer_line(wait_for_user_input("Press enter to continue to the next game"))
        }
    }

    vector<Team*> final_standings(teams);
    sort(final_standings.begin(), final_standings.end(), [](const Team* a, const Team* b){return a->wins > b->wins;});
    return final_standings;
}


eTeam Season::simulate_matchup(const Matchup& matchup) {
    matchup.home_team->prepare_for_game(matchup.day_of_year, true);
    matchup.away_team->prepare_for_game(matchup.day_of_year, true);

    Baseball_Game game = matchup.load_game();
    Game_Result result = game.play_game();

    for (Player* player : result.home_team->pitchers_used) {
        player->day_of_last_game_played = matchup.day_of_year;
    }
    for (Player* player : result.away_team->pitchers_used) {
        player->day_of_last_game_played = matchup.day_of_year;
    }

    return result.winner;
}


Matchup::Matchup(Team* home_team, Team* away_team, uint day_of_year) {
    this->home_team = home_team;
    this->away_team = away_team;
    this->day_of_year = day_of_year;
}