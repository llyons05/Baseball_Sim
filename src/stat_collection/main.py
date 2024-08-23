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
        handle_data_viewing()


def handle_user_scraping() -> None:
    scraping_mode = UI.get_scraping_mode()

    if scraping_mode == "roster":
        handle_roster_scraping()

    elif scraping_mode == "stats":
        handle_player_stats_scraping()

def handle_roster_scraping():
    rosters_to_scrape = UI.get_roster_scraping_selection()
    year = UI.choose_year()
    save_rosters_from_year(rosters_to_scrape, year)


def save_rosters_from_year(teams: list[str], year: str) -> None:
    all_teams = DI.get_all_teams()
    year = str(year)
    overwrite_data = int(utils.get_current_year()) == int(year)

    for team_name, team_url, team_abbreviation in all_teams:
        if team_abbreviation in teams:
            scrape_and_save_team_roster(team_url, team_abbreviation, year, overwrite_data)


def scrape_and_save_team_roster(team_url: str, team_abbreviation: str, year: str, overwrite_data: bool = True) -> bool:
    client = Scraping_Client()

    year_page = client.scrape_team_page_for_roster_url(team_url, year)
    if year_page != None:
        print(f"Saving {year} {team_abbreviation} roster...")

        if (not DI.team_roster_file_exists(team_abbreviation, year)) or overwrite_data:
            DI.create_team_year_folder(team_abbreviation, year)
            roster = client.scrape_team_roster_page(year_page)
            DI.save_team_roster_file(team_abbreviation, year, roster)
        return True
    else:
        return False


def handle_player_stats_scraping() -> None:
    teams_to_scrape = UI.get_team_stats_scraping_selection()
    year = UI.choose_year()
    stat_type = UI.choose_player_stat_type()

    for team in teams_to_scrape:
        save_all_team_player_data(team, year, stat_type)


def save_all_team_player_data(team_abbreviation: str, year: str, stat_type: DI.STAT_TYPES) -> None:
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
            save_player_data(player["URL"], player["ID"], stat_type, overwrite_data)


def save_player_data(player_page_url: str, player_id: str, stat_type: DI.STAT_TYPES, overwrite_data: bool = True) -> None:
    print(f"Saving {player_id} {stat_type} stats...")
    client = Scraping_Client()

    if (not DI.player_data_file_exists(player_id, stat_type)) or overwrite_data:
        player_data = client.scrape_player_stats(player_page_url, stat_type)
        DI.save_player_data_file(player_id, stat_type, player_data)


def handle_data_viewing() -> None:
    team = UI.get_single_team_choice()
    year = UI.choose_year()

    if not DI.team_roster_file_exists(team, year):
        handle_missing_team_roster_file(team, year)
    
    player = UI.get_player_choice(team, year)
    stat_type = UI.choose_viewing_stat_type()

    if not DI.player_data_file_exists(player, stat_type):
        handle_missing_player_data_file(player, stat_type, team, year)

    UI.display_player_data_table(player, stat_type)


def handle_missing_team_roster_file(team_abbreviation: str, year: str) -> None:
    if UI.should_download_missing_team_roster_file(team_abbreviation, year):
        all_teams = DI.get_all_teams()
        for team in all_teams:
            if team[2] == team_abbreviation:
                scrape_and_save_team_roster(team[1], team[2], year)


def handle_missing_player_data_file(player_id: str, stat_type: DI.STAT_TYPES, team_abbreviation: str, year: str) -> None:
    if UI.should_download_missing_player_data_file(player_id, stat_type):
        team_data = DI.read_team_roster_file(team_abbreviation, year)
        for team_player in team_data:
            if team_player["ID"] == player_id:
                save_player_data(team_player["URL"], team_player["ID"], stat_type)
    else:
        quit()


if __name__ == "__main__":
    main()