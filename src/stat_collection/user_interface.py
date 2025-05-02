import math
from typing import Literal
from termcolor import colored

import local_database_interface as DI
import utils

def get_user_mode() -> Literal["scrape", "view", "audit"]:
    prompt = f"{colored('scrape', 'green')} data, {colored('view', 'red')} data, or {colored('audit', 'blue')}?"
    mode = get_user_choice_from_prompt(prompt, ["s", "v", "a"]).lower()

    if mode == "s":
        return "scrape"
    if mode == "v":
        return "view"
    return "audit"


def get_scraping_mode() -> Literal["team", "player", "league"]:
    prompt = f"scrape {colored('team statistics', 'green')} or {colored('player statistics', 'red')} or {colored('league statistics', 'blue')}?"
    mode: str = get_user_choice_from_prompt(prompt, ["t", "p", "l"]).lower()
    
    if mode == "t":
        return "team"
    elif mode == "p":
        return "player"
    return "league"


def get_audit_mode() -> Literal["team", "season"]:
    prompt = "Audit team or season?"
    mode = get_user_choice_from_prompt(prompt, ["team", "season"]).lower()
    return mode


def get_team_data_scraping_selection() -> list[str]:
    roster_prompt_text = "Input the abbreviations for the teams whose team data should be saved."
    roster_example_text = f"Press enter to save all teams, or type teams comma separated (ex: {colored('NYY', 'blue')}, {colored('WSN', 'red')}, {colored('PIT', 'yellow')}): "

    all_teams = [x["TEAM_ID"] for x in DI.get_all_teams()]
    roster_input_str = ""

    display_all_teams_table()
    while not validate_user_string_input_list(roster_input_str, 3, 3, allowed_values=all_teams):
        roster_input_str = input(f"{roster_prompt_text}\n{roster_example_text}").upper()

        if not roster_input_str.strip():
            return all_teams
    
    print()
    # using list(set()) removes duplicates
    return list(set(parse_user_string_input_list(roster_input_str)))


def get_team_player_stats_scraping_selection() -> list[str]:
    roster_prompt_text = "Input the abbreviations for the teams whose players' stats should be saved."
    roster_example_text = f"Press enter to save all teams, or type comma separated teams (ex: {colored('NYY', 'blue')}, {colored('WSN', 'red')}, {colored('PIT', 'yellow')}): "

    all_teams = [team["TEAM_ID"] for team in DI.get_all_teams()]
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


def choose_league_scraping_stat_types() -> list[DI.LEAGUE_DATA_FILE_TYPES]:
    prompt = "What league stats should be scraped?"
    return get_user_choices_from_prompt(prompt, list(DI.get_league_data_file_types()))


def choose_team_scraping_stat_types() -> list[DI.TEAM_DATA_FILE_TYPES]:
    prompt = "What team stats should be scraped?"
    return get_user_choices_from_prompt(prompt, list(DI.get_team_data_file_types()))


def choose_player_scraping_stat_types() -> list[DI.PLAYER_STAT_TYPES]:
    prompt = "What player stats should be scraped?"
    return get_user_choices_from_prompt(prompt, list(DI.get_player_stat_types()))


def should_overwrite_data() -> bool:
    prompt = "Should previously saved data be used when available?"
    return get_yes_or_no(prompt, True)


def should_gather_non_pitcher_stats() -> bool:
    prompt = "Should non-pitchers have their pitching stats gathered?"
    return get_yes_or_no(prompt)


def validate_user_string_input_list(input_str: str, min_single_value_size: int = -1, max_single_value_size: int = math.inf, delimiter: str = ",", allowed_values: list[str] = []) -> bool:
    input_str = input_str.strip()
    input_list = parse_user_string_input_list(input_str)

    if not input_str:
        return False
    
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
    for i in range(len(input_list)):
        input_list[i] = input_list[i].strip()

    return input_list


def get_single_team_choice() -> str:
    team_choice = ""
    all_teams = [team["TEAM_ID"] for team in DI.get_all_teams()]

    display_all_teams_table()
    while team_choice not in all_teams:
        team_choice = input("Input team abbreviation: ").upper()
    
    print()
    return team_choice


def get_player_choice(team_abbreviation: str, year: int) -> None:
    display_team_roster_file(team_abbreviation, year)

    player_choice = ""
    all_players = [player_data["ID"] for player_data in DI.read_team_data_file(team_abbreviation, year, "roster")]

    while player_choice not in all_players:
        player_choice = input("Input the ID of the player you want to view: ")
    
    print()
    return player_choice


def choose_player_viewing_stat_type() -> DI.PLAYER_STAT_TYPES:
    prompt = "What player stats should be viewed?"
    return get_user_choice_from_prompt(prompt, list(DI.get_player_stat_types()))


def display_all_teams_table() -> None:
    utils.print_csv(f"{DI.RESOURCES_DIR}/all_teams.csv")
    print()


def display_player_data_table(player_id: str, stat_type: DI.PLAYER_STAT_TYPES) -> None:
    filename = DI.get_player_data_file_path(player_id, stat_type)
    utils.print_csv(filename)
    print()


def display_team_roster_file(team_abbreviation: str, year: int) -> None:
    filename = DI.get_team_data_file_path(team_abbreviation, year, "roster")
    headers_to_display = ["name_display", "pos", "ID"]
    utils.print_csv(filename, headers_to_display)
    print()


def get_yes_or_no(prompt: str, inverted: bool = False) -> bool:
    result = get_user_choice_from_prompt(prompt, ["y", "n"])[0]

    if not inverted:
        return result == "y"

    return result == "n"


def get_user_choice_from_prompt(prompt: str, choices: list[str]) -> str:
    choice = ""
    prompt_str = get_user_choice_prompt_str(prompt, choices)

    while choice.lower() not in choices:
        choice = input(prompt_str)
    
    print()
    return choice


def get_user_choices_from_prompt(prompt: str, choices: list[str], default: list[str] = None) -> list[str]:
    user_input = ""
    prompt += " Press enter for all or select from"
    prompt_str = get_user_choice_prompt_str(prompt, choices)

    while not validate_user_string_input_list(user_input, allowed_values=choices):
        user_input = input(prompt_str)
        if not user_input.strip():
            if default == None:
                default = choices
            return default
    
    print()
    return parse_user_string_input_list(user_input)


def get_user_choice_prompt_str(prompt: str, choices: list[str]) -> str:
    result = prompt + " ("
    colors = ["green", "red", "blue", "yellow"]
    for i, choice in enumerate(choices):
        result += colored(choice, colors[i%4])
        if i < len(choices)-1:
            result += "/"
    return result + "): "


def wait_for_user_input(prompt: str = "") -> str:
    return input(prompt)