from baseball_reference_client import Scraping_Client
import local_database_interface as DI
import user_interface as UI
import utils

def main():
    if not DI.is_file_structure_set_up():
        print("Setting up data structure...")
        DI.set_up_file_structure()
    
    usermode = UI.get_user_mode()

    if usermode == "scrape":
        handle_user_scraping()
    
    elif usermode == "view":
        print("Not implemented.")


def handle_user_scraping() -> None:
    scraping_mode = UI.get_scraping_mode()

    if scraping_mode == "roster":
        handle_roster_scraping()

    elif scraping_mode == "stats":
        handle_stats_scraping()

def handle_roster_scraping():
    rosters_to_scrape = UI.get_roster_scraping_selection()
    year = UI.choose_year()
    save_rosters_from_year(rosters_to_scrape, year)


def save_rosters_from_year(teams: list[str], year: str) -> None:
    client = Scraping_Client()
    all_teams = DI.get_all_teams()
    year = str(year)
    overwrite_data = int(utils.get_current_year()) == int(year)

    for team_name, team_url, team_abbreviation in all_teams:
        if team_abbreviation in teams:
            year_page = client.scrape_team_page_for_roster_url(team_url, year)

            if year_page != None:
                print(f"Saving {year} {team_name} roster...")

                if (not DI.team_roster_file_exists(team_abbreviation, year)) or overwrite_data:
                    DI.create_team_year_folder(team_abbreviation, year)
                    roster = client.scrape_team_roster_page(year_page)
                    DI.save_team_roster_file(team_abbreviation, year, roster)


def handle_stats_scraping() -> None:
    teams_to_scrape = UI.get_team_stats_scraping_selection()
    year = UI.choose_year()
    stat_type = UI.choose_player_stat_type()

    for team in teams_to_scrape:
        save_all_team_player_data(team, year, stat_type)


def save_all_team_player_data(team_abbreviation: str, year: str, stat_type: DI.STAT_TYPES) -> None:
    client = Scraping_Client()
    overwrite_data = UI.should_overwrite_data()

    if stat_type == "pitching":
        should_gather_non_pitchers = UI.should_gather_non_pitcher_stats()
    else:
        should_gather_non_pitchers = True

    if not DI.team_roster_file_exists(team_abbreviation, year):
        print(f"{team_abbreviation} {year} roster file not found locally, scraping baseball reference...")
        save_rosters_from_year([team_abbreviation], year)
        print("continuing...")


    team_roster = DI.read_team_roster_file(team_abbreviation, int(year))

    print(f"Saving {team_abbreviation} {year} roster player {stat_type} stats...")

    for player in team_roster:
        if (player["POS"] == "P") or should_gather_non_pitchers:
            url = player["URL"]
            name = player["NAME"].replace(";", ", ")

            print(f"Saving {name} {stat_type} stats...")

            if (not DI.player_data_file_exists(player["ID"], stat_type)) or overwrite_data:
                batting_data = client.scrape_player_stats(url, stat_type)
                DI.save_player_year_data(player["ID"], stat_type, batting_data)


if __name__ == "__main__":
    main()