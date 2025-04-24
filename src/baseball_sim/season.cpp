#include "season.hpp"

#include "utils.hpp"
#include "player.hpp"
#include "team.hpp"
#include "baseball_game.hpp"

#include <vector>
#include <string>
#include <algorithm>

using namespace std;


Season::Season(const vector<Team*>& teams, unsigned int year) {
    this->teams = teams;
    this->year = year;

    populate_matchups();
}


void Season::populate_matchups() {
    vector<Team*> loaded_teams;
    for (Team* team : teams) {
        for (unsigned int i = 0; i < team->team_stats[TEAM_SCHEDULE].size(); i++) {
            Team* away_team = team_cache.at(get_team_cache_id(team->team_stats.get_stat<string>(TEAM_SCHEDULE, "opp_ID", i, "NO TEAM FOUND"), year)).get();

            if (find(loaded_teams.begin(), loaded_teams.end(), away_team) != loaded_teams.end()) {
                unsigned int day_of_year = 0; // TODO: Get day of year from matchup
                matchups.push_back(Matchup(team, away_team, day_of_year));
            }
        }
        loaded_teams.push_back(team);
    }

    sort(matchups.begin(), matchups.end(), [](const Matchup& a, const Matchup& b){return a.day_of_year < b.day_of_year;});
    cout << "Num Matchups: "<< matchups.size() << "\n";
}


// Return the teams in order of win %
vector<Team*> Season::run_games(unsigned int sims_per_matchup) {
    for (Matchup& matchup : matchups) {
        eTeam winner = simulate_matchup(matchup, sims_per_matchup);
        if (winner == HOME_TEAM) {
            matchup.home_team->wins++;
            matchup.away_team->losses++;
        }
        else {
            matchup.away_team->wins++;
            matchup.home_team->losses++;
        }
    }

    vector<Team*> final_standings(teams);
    sort(final_standings.begin(), final_standings.end(), [](const Team* a, const Team* b){return a->wins > b->wins;});
    return final_standings;
}


eTeam Season::simulate_matchup(const Matchup& matchup, unsigned int num_simulations) {
    unsigned int wins[2] = {0, 0};
    for (unsigned int i = 0; i < num_simulations; i++) {
        Baseball_Game game = matchup.load_game();
        Game_Result result = game.play_game();
        wins[result.winner]++;

        matchup.home_team->reset();
        matchup.away_team->reset();
    }

    if (wins[HOME_TEAM] >= wins[AWAY_TEAM]) return HOME_TEAM;
    return AWAY_TEAM;
}


Matchup::Matchup(Team* home_team, Team* away_team, unsigned int day_of_year) {
    this->home_team = home_team;
    this->away_team = away_team;
    this->day_of_year = day_of_year;
}