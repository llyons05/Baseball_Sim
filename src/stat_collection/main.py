import tqdm

import local_database_interface as DI
import user_interface as UI
import all_data_handler as Data_Handler
import utils

def main():
    if not DI.is_file_structure_set_up():
        print("Setting up data structure...")
        DI.set_up_file_structure()

    while True:
        usermode = UI.get_user_mode()

        if usermode == "scrape":
            handle_user_scraping()
        
        elif usermode == "view":
            handle_data_viewing()

        elif usermode == "audit":
            handle_data_audit()


def handle_user_scraping() -> None:
    scraping_mode = UI.get_scraping_mode()

    if scraping_mode == "team":
        handle_team_data_scraping()

    elif scraping_mode == "player":
        handle_player_stats_scraping()

    elif scraping_mode == "league":
        handle_league_stats_scraping()


def handle_league_stats_scraping():
    overwrite_data = UI.should_overwrite_data()
    Data_Handler.scrape_and_save_league_data(overwrite_data)
    UI.wait_for_user_input("Done. Press enter to continue.")


def handle_team_data_scraping():
    teams_to_scrape = UI.get_roster_scraping_selection()
    year = UI.choose_year()
    scrape_and_save_teams_data(teams_to_scrape, year)
    UI.wait_for_user_input("Done. Press enter to continue.")


def scrape_and_save_teams_data(teams: list[str], year: int) -> None:
    all_teams = DI.get_all_teams()
    overwrite_data = utils.get_current_year() == year

    for team_data in all_teams:
        if team_data["TEAM_ID"] in teams:
            Data_Handler.scrape_and_save_team_data(team_data["URL"], team_data["TEAM_ID"], year, overwrite_data)


def scrape_and_save_single_team_data(team: str, year: int) -> bool:
    all_teams = DI.get_all_teams()
    overwrite_data = utils.get_current_year() == year

    for team_data in all_teams:
        if team_data["TEAM_ID"] == team:
            return Data_Handler.scrape_and_save_team_data(team_data["URL"], team_data["TEAM_ID"], year, overwrite_data)
    
    return False


def handle_player_stats_scraping() -> None:
    teams_to_scrape = UI.get_team_stats_scraping_selection()
    year = UI.choose_year()
    stat_types = UI.choose_player_scraping_stat_types()
    overwrite_data = UI.should_overwrite_data()

    for team in teams_to_scrape:
        save_all_team_player_data(team, year, stat_types, overwrite_data)
    
    UI.wait_for_user_input("Done. Press enter to continue.")


def save_all_team_player_data(team_abbreviation: str, year: int, stat_types: list[DI.PLAYER_STAT_TYPES], overwrite_data: bool = True) -> None:

    if not handle_missing_team_data_file(team_abbreviation, year):
        return

    player_list = DI.get_player_list(team_abbreviation, year, stat_types)
    print(f"Saving {team_abbreviation} {year} roster player {"/".join(stat_types)} stats...")

    progress_bar = tqdm.tqdm(player_list, unit="player")
    for player in progress_bar:
        progress_bar.set_description_str(player["ID"])
        Data_Handler.scrape_and_save_player_data(utils.BASE_URL + player["URL"], player["ID"], player["STAT_TYPES"], overwrite_data)


def handle_data_viewing() -> None:
    team = UI.get_single_team_choice()
    year = UI.choose_year()

    if not handle_missing_team_data_file(team, year):
        return
    
    player = UI.get_player_choice(team, year)
    stat_type = UI.choose_player_viewing_stat_type()

    if not handle_missing_player_data_file(player, stat_type, team, year):
        return

    UI.display_player_data_table(player, stat_type)
    UI.wait_for_user_input("Press enter to view a different player's stats.")


def handle_missing_team_data_file(team_abbreviation: str, year: int) -> bool:
    missing_data_file = DI.find_missing_team_data_files(team_abbreviation, year)

    if missing_data_file:

        print(f"{team_abbreviation} {year} {missing_data_file} file not found locally, scraping baseball reference...")

        data_successfully_found = scrape_and_save_single_team_data(team_abbreviation, year)
        if not data_successfully_found:
            UI.wait_for_user_input(f"There was an error finding a {year} {team_abbreviation} roster file. Are you sure {team_abbreviation} existed in {year}?. No data was saved")
            return False

        print("continuing...")
    
    return True



def handle_missing_player_data_file(player_id: str, stat_type: DI.PLAYER_STAT_TYPES, team_abbreviation: str, year: int) -> bool:
    if not DI.player_data_file_exists(player_id, stat_type):
        print(f"There was no local {stat_type} file for {player_id}, scraping baseball reference...")
        team_data = DI.read_team_data_file(team_abbreviation, year, "roster")
        search_results = team_data.search_rows({"ID": player_id})
        data_successfully_found = False
        if search_results:
            player = search_results[0]
            data_successfully_found = Data_Handler.scrape_and_save_player_data(utils.BASE_URL + player["URL"], player["ID"], [stat_type])

        if not data_successfully_found:
            UI.wait_for_user_input(f"There was an error finding {stat_type} data for {player_id}. No data was saved.")
            return False

        print("continuing...")

    return True


def handle_data_audit() -> None:
    team = UI.get_single_team_choice()
    year = UI.choose_year()
    audit_team(team, year)
    UI.wait_for_user_input("Press enter to continue.")


def audit_team(team_abbreviation: str, year: int) -> None:
    audit_str = f"Local Database Audit Results for {team_abbreviation}-{year}:\n"
    missing_team_files, team_file_audit_str = audit_team_files(team_abbreviation, year)
    if len(missing_team_files) > 0: # Cannot continue with the audit if we do not have all the team files
        print(audit_str + team_file_audit_str)
        return
    
    missing_roster_files, roster_audit_str = audit_team_roster_files(team_abbreviation, year)
    if len(missing_roster_files) == 0:
        print(audit_str + f"\tAudit of {team_abbreviation}-{year} successful, no missing files.\n")
        return
    
    short_audit_str = audit_str + f"\tAUDIT FAILED:\n\t{len(missing_roster_files)} missing files.\n"
    print(short_audit_str)

    display_extended_audit = UI.get_yes_or_no("Would you like to see an extended summary?")
    if display_extended_audit:
        print(roster_audit_str)
    
    return


def audit_team_files(team_abbreviation: str, year: int) -> tuple[list[str], str]:
    """ Return (list of missing files, description of audit findings) """
    if not DI.team_parent_dir_exists(team_abbreviation):
        return [DI.get_team_dir_path(team_abbreviation)], f"\tMissing main team parent directory at {DI.get_team_dir_path(team_abbreviation)}. Run local_database_interface.py to fix this.\n"
    
    if not DI.team_year_dir_exists(team_abbreviation, year):
        return [DI.get_team_year_dir_path(team_abbreviation, year)], f"\tMissing team year directory at {DI.get_team_year_dir_path(team_abbreviation, year)}. Try scraping this team's data again.\n"
    
    missing_files = []
    result_str = ""
    for file_type in DI.get_team_data_file_types():
        if not DI.team_data_file_exists(team_abbreviation, year, file_type):
            missing_files.append(DI.get_team_data_file_path(team_abbreviation, year, file_type))
            result_str += f"\tMissing team {file_type} file.\n"
    
    return missing_files, result_str


def audit_team_roster_files(team_abbreviation: str, year: int) -> tuple[list[str], str]:
    """ Return (list of missing files, description of audit findings) """
    team_roster = DI.get_player_list(team_abbreviation, year, DI.get_player_stat_types())

    missing_files, audit_str = [], ""
    for player in team_roster:
        player_missing_files, player_audit_str = audit_player(player["ID"], player["STAT_TYPES"])
        missing_files.extend(player_missing_files)
        audit_str += player_audit_str

    return missing_files, audit_str


def audit_player(player_id: str, stat_types: DI.PLAYER_STAT_TYPES) -> tuple[list[str], str]:
    """ Return (list of missing files, description of audit findings) """
    missing_files, audit_str = [], ""
    for stat_type in stat_types:
        if not DI.player_data_file_exists(player_id, stat_type):
            missing_files.append(DI.get_player_data_file_path(player_id, stat_type))
            audit_str += f"\tMissing {stat_type} file for {player_id}.\n"
    
    return missing_files, audit_str


if __name__ == "__main__":
    main()