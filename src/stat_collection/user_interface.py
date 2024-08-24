import os, sys, tqdm, pprint, math
from typing import Literal
from termcolor import cprint, COLORS, colored

import local_database_interface as DI
import utils

def get_user_mode() -> Literal["scrape", "view"]:
    mode: str = ""
    while mode not in ("s", "v"):
        print(f"{colored("scrape", "green")} data or {colored("view", "red")} data? ({colored("S", "green")}/{colored("v", "red")})", end=": ")
        mode = input().lower()
    print()
    
    if mode == "s":
        return "scrape"
    
    return "view"


def get_scraping_mode() -> Literal["roster", "stats"]:
    mode: str = ""
    while mode.lower() not in ("r", "s"):
        print(f"scrape {colored("team roster", "green")} or {colored("player statistics", "red")}? ({colored("R", "green")}/{colored("s", "red")})", end=": ")
        mode = input().lower()
    print()
    
    if mode == "r":
        return "roster"
    
    return "stats"


def get_roster_scraping_selection() -> list[str]:
    roster_prompt_text = "Input the abbreviations for the teams whose rosters should be saved."
    roster_example_text = f"Press enter to save all teams, or type teams comma separated (ex: {colored("NYY", "blue")}, {colored("WSN", "red")}, {colored("PIT", "yellow")}): "

    all_teams = [x[2] for x in DI.get_all_teams()]
    roster_input_str = ""

    display_all_teams_table()
    while not validate_user_string_input_list(roster_input_str, 3, 3, allowed_values=all_teams):
        roster_input_str = input(f"{roster_prompt_text}\n{roster_example_text}").upper()

        if not roster_input_str.strip():
            return all_teams
    
    print()
    # using list(set()) removes duplicates
    return list(set(parse_user_string_input_list(roster_input_str)))


def get_team_stats_scraping_selection() -> list[str]:
    roster_prompt_text = "Input the abbreviations for the teams whose players' stats should be saved."
    roster_example_text = f"Press enter to save all teams, or type comma separated teams (ex: {colored("NYY", "blue")}, {colored("WSN", "red")}, {colored("PIT", "yellow")}): "

    all_teams = [team[2] for team in DI.get_all_teams()]
    roster_input_str = ""

    display_all_teams_table()
    while not validate_user_string_input_list(roster_input_str, 3, 3, allowed_values=all_teams):
        roster_input_str = input(f"{roster_prompt_text}\n{roster_example_text}").upper()

        if not roster_input_str.strip():
            return all_teams
    
    print()
    # using list(set()) removes duplicates
    return list(set(parse_user_string_input_list(roster_input_str))) 


def choose_year() -> int:
    year_input = "abc"
    while (not year_input.isdigit()) or (int(year_input) > int(utils.get_current_year())):
        year_input = input("Input year to get data from: ")
    print()

    return int(year_input)


def choose_player_stat_type() -> DI.STAT_TYPES:
    stat_type = ""
    while stat_type not in ("batting", "pitching"):
        stat_type = input(f"What player stats should be scraped? ({colored("batting", "red")}, {colored("pitching", "blue")}): ")
    
    print()
    return stat_type


def should_overwrite_data() -> bool:
    answer: str = ""
    while answer.lower() not in ("y", "n"):
        print(f"Should previously saved data be used when available? ({colored("Y", "green")}/{colored("n", "red")})", end=": ")
        answer = input().lower()
    print()
    
    if answer == "y":
        return False
    
    return True


def should_gather_non_pitcher_stats() -> bool:
    answer: str = ""
    while answer.lower() not in ("y", "n"):
        print(f"Should non-pitchers have their pitching stats gathered? ({colored("Y", "green")}/{colored("n", "red")})", end=": ")
        answer = input().lower()
    print()
    
    if answer == "y":
        return True

    return False


def validate_user_string_input_list(input_str: str, min_single_value_size: int = -1, max_single_value_size: int = math.inf, delimiter: str = ",", allowed_values: list[str] = []) -> bool:
    input_str = input_str.strip()
    input_list = parse_user_string_input_list(input_str)
    
    if (len(input_str.replace(delimiter, "")) < min_single_value_size):
        print()
        return False
    
    if (len(input_list) == 1) and ((len(input_list[0]) < min_single_value_size) or (len(input_list[0]) > max_single_value_size)):
        print(colored(f"\n{input_str} is not a valid input\n", "light_red"))
        return False

    for value in input_list:
        if value not in allowed_values:
            print(colored(f"\n{value} is not a valid input\n", "light_red"))
            return False

    return True
    

def parse_user_string_input_list(input_str: str, delimiter: str = ",") -> list[str]:
    input_list = input_str.split(delimiter)
    for x in range(len(input_list)):
        input_list[x] = input_list[x].strip()

    return input_list


def get_single_team_choice() -> str:
    team_choice = ""
    all_teams = [team[2] for team in DI.get_all_teams()]

    display_all_teams_table()
    while team_choice not in all_teams:
        team_choice = input("Input team abbreviation: ").upper()
    
    print()
    return team_choice


def get_player_choice(team_abbreviation: str, year: int) -> None:
    display_team_roster_file(team_abbreviation, year)

    player_choice = ""
    all_players = [player_data["ID"] for player_data in DI.read_team_roster_file(team_abbreviation, year)]

    while player_choice not in all_players:
        player_choice = input("Input the ID of the player you want to view: ")
    
    print()
    return player_choice

def choose_viewing_stat_type() -> DI.STAT_TYPES:
    stat_type = ""
    while stat_type not in ("batting", "pitching"):
        stat_type = input(f"What player stats should be viewed? ({colored("batting", "red")}, {colored("pitching", "blue")}): ")
    
    print()
    return stat_type


def should_download_missing_team_roster_file(team_abbreviation: str, year: int) -> bool:
    print(f"{colored("ERROR: It looks like a", "red")} {colored(f"{year} {team_abbreviation}", "green")} {colored("roster file does not exist locally.", "red")}\n")

    user_input = ""
    while user_input not in ("y", "n"):
        user_input = input(f"Would you like to scrape it from baseball reference? ({colored("Y", "green")}/{colored("n", "red")}): ").lower()

    if user_input == "y":
        return True
    
    return False


def should_download_missing_player_data_file(player_id: str, stat_type: DI.STAT_TYPES) -> bool:
    print(f"{colored("ERROR: It looks like", "red")} {colored(stat_type, "green")} {colored("data for", "red")} {colored(player_id, "green")} {colored("does not exist locally.", "red")}\n")

    user_input = ""
    while user_input not in ("y", "n"):
        user_input = input(f"Would you like to scrape it from baseball reference? ({colored("Y", "green")}/{colored("n", "red")}): ").lower()

    if user_input == "y":
        return True
    
    return False


def display_all_teams_table() -> None:
    utils.print_csv(f"{DI.RESOURCES_DIR}/all_teams.csv")
    print()


def display_player_data_table(player_id: str, stat_type: DI.STAT_TYPES) -> None:
    filename = DI.get_player_data_file_path(player_id, stat_type)
    utils.print_csv(filename)
    print()


def display_team_roster_file(team_abbreviation: str, year: int) -> None:
    filename = DI.get_team_roster_file_path(team_abbreviation, year)
    utils.print_csv(filename)
    print()


def wait_for_user_input(prompt: str = "") -> str:
    return input(prompt)