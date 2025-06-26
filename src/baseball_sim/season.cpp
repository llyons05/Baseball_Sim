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
            const std::string opponent_abbr = schedule_table.get_stat<string>("opp_ID", i, "");
            Team* opponent_team = team_cache.at(get_team_cache_id(opponent_abbr, year)).get();

            if (find(loaded_teams.begin(), loaded_teams.end(), opponent_team) != loaded_teams.end()) {
                uint day_of_year = get_day_of_year(schedule_table.get_stat<string>("date_game", i, ""), year);
                bool is_home_game = schedule_table.get_stat<string>("homeORvis", i, "") == "";
                Team* home_team = is_home_game? team : opponent_team;
                Team* away_team = is_home_game? opponent_team : team;
                matchups.push_back(Matchup(home_team, away_team, day_of_year));
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
                matchup.home_team->running_stats.wins++;
                matchup.away_team->running_stats.losses++;
            }
            else {
                matchup.away_team->running_stats.wins++;
                matchup.home_team->running_stats.losses++;
            }

            game_viewer_line(wait_for_user_input("Press enter to continue to the next game"))
        }
    }

    vector<Team*> final_standings(teams);
    sort(final_standings.begin(), final_standings.end(), [](const Team* a, const Team* b){return a->running_stats.wins > b->running_stats.wins;});
    return final_standings;
}


eTeam Season::simulate_matchup(Matchup& matchup) {
    matchup.home_team->prepare_for_game(matchup.day_of_year, true);
    matchup.away_team->prepare_for_game(matchup.day_of_year, true);

    Game_Result result = matchup.play();
    return result.winner;
}


Matchup::Matchup(Team* home_team, Team* away_team, uint day_of_year) {
    this->home_team = home_team;
    this->away_team = away_team;
    this->day_of_year = day_of_year;
}


Series::Series(Team* home_team, Team* away_team, uint games_in_series, uint num_simulations) {
    teams[HOME_TEAM] = home_team;
    teams[AWAY_TEAM] = away_team;
    this->games_in_series = games_in_series;
    this->num_simulations = num_simulations;
    games_to_clinch = games_in_series/2 + 1;

    if (games_in_series * num_simulations == 0) {
        cerr << "Please input a nonzero number of games.\n";
        throw exception();
    }
    populate_matchups();
}


void Series::populate_matchups() {
    for (uint i = 0; i < games_in_series; i++) {
        matchups.push_back(get_series_matchup(i));
    }
}


Matchup Series::get_series_matchup(uint current_matchup_index) {
    if (games_in_series < 5) {
        return Matchup(teams[HOME_TEAM], teams[AWAY_TEAM], current_matchup_index);
    }
    bool is_home_advantage = (current_matchup_index < 2) || (current_matchup_index >= 2 + games_in_series/2);
    Team* home_team = is_home_advantage ? teams[HOME_TEAM] : teams[AWAY_TEAM];
    Team* away_team = is_home_advantage ? teams[AWAY_TEAM] : teams[HOME_TEAM];
    uint day = current_matchup_index + ((current_matchup_index >= 2) ? 1 : 0) + ((current_matchup_index >= 2 + games_in_series/2) ? 1 : 0);
    return Matchup(home_team, away_team, day);
}


// Returns the team that won the series the most often
eTeam Series::play() {
    for (uint i = 0; i < num_simulations; i++) {
        eTeam winner = play_series_once();
        series_won[winner]++;
        teams[HOME_TEAM]->reset_player_tracking_data();
        teams[AWAY_TEAM]->reset_player_tracking_data();
    }
    return (series_won[HOME_TEAM] >= series_won[AWAY_TEAM]) ? HOME_TEAM : AWAY_TEAM; 
}


eTeam Series::play_series_once() {
    uint games_won[2] = {0, 0};
    uint games_played = 0;

    for (Matchup& matchup : matchups) {
        Game_Result result = matchup.play();
        Team* winning_team = (result.winner == HOME_TEAM) ? result.home_team : result.away_team;
        eTeam winner = (winning_team == teams[HOME_TEAM]) ? HOME_TEAM : AWAY_TEAM;
        
        games_played++;
        games_won[winner]++;

        if ((games_won[HOME_TEAM] >= games_to_clinch) || (games_won[AWAY_TEAM] >= games_to_clinch)) break;
    }

    eTeam winner = (games_won[HOME_TEAM] >= games_won[AWAY_TEAM]) ? HOME_TEAM : AWAY_TEAM; // If we are in a series with an even number of games and the teams tie, we just return the home team because why not
    games_played_in_series_won[winner] += games_played;
    total_games_played += games_played;
    return winner; 
}


void Series::print_results() {
    for (uint i = 0; i < games_in_series; i++) {
        cout << std::fixed << std::setprecision(1);
        cout << "GAME " << i+1 << " (" << 100.f*matchups[i].times_played/num_simulations << "% played):\n";
        matchups[i].print_results();
    }
    cout << "\nSERIES RESULTS:\n";
    cout << "\t" << teams[HOME_TEAM]->team_name << "\t" << teams[AWAY_TEAM]->team_name << "\n";
    cout << "Win%   " << (float)series_won[HOME_TEAM]/num_simulations << "\t" << (float)series_won[AWAY_TEAM]/num_simulations << "\n";
    cout << "G/Win  " << (float)games_played_in_series_won[HOME_TEAM] / (series_won[HOME_TEAM]? series_won[HOME_TEAM]:1) << "\t" << (float)games_played_in_series_won[AWAY_TEAM] / (series_won[AWAY_TEAM]? series_won[AWAY_TEAM]:1) << "\n\n";
}


void Matchup::print_results() {
    cout << std::fixed << std::setprecision(3);
    cout << "\t" << away_team->team_name << "  @\t" << home_team->team_name << "\n";
    cout << "Win%  " << (float)games_won[AWAY_TEAM]/times_played << "\t" << (float)games_won[HOME_TEAM]/times_played << "\n";
    cout << "Runs  " << (float)runs_scored[AWAY_TEAM]/times_played << "\t" << (float)runs_scored[HOME_TEAM]/times_played << "\n";
    cout << "\n";
}