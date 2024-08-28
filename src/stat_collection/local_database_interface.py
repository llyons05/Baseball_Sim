import os, sys, time, csv
from typing import Literal, get_args

import utils

TEAMS_DIR: str = "data/teams"
PLAYERS_DIR: str = "data/players"
RESOURCES_DIR: str = "resources"

STAT_TYPES = Literal["batting", "pitching"]
TEAM_DATA_FILE_TYPES = Literal["roster", "pitching"]


def set_up_file_structure():
    utils.make_dirs("data")
    utils.make_dirs(PLAYERS_DIR)
    utils.make_dirs(f"{PLAYERS_DIR}/batting")
    utils.make_dirs(f"{PLAYERS_DIR}/pitching")

    create_all_team_folders()


def is_file_structure_set_up() -> bool:
    return os.path.exists(TEAMS_DIR) or os.path.exists(PLAYERS_DIR)


def create_all_team_folders():
    utils.make_dirs(TEAMS_DIR)

    for team_name, team_url, team_abbrev in get_all_teams():
        create_team_folder(team_abbrev)


def create_team_folder(team_abbreviation: str) -> None:
    dir_path: str = get_team_dir_path(team_abbreviation)
    utils.make_dirs(dir_path)


def create_team_year_folder(team_abbreviation: str, year: int) -> None:
    dir_path: str = get_team_year_dir_path(team_abbreviation, year)
    utils.make_dirs(dir_path)


def save_team_roster_file(team_abbreviation: str, year: int, roster: list[tuple]) -> None:
    headers = ["FIRSTNAME", "LASTNAME", "ID", "POS", "URL"]
    save_team_data_file(team_abbreviation, year, roster, headers, "roster")


def save_team_pitching_file(team_abbreviation: str, year: int, pitchers: list[tuple]) -> None:
    headers = ["FIRSTNAME", "LASTNAME", "ID", "POS", "URL"]
    save_team_data_file(team_abbreviation, year, pitchers, headers, "pitching")


def save_team_data_file(team_abbreviation: str, year: int, data: list[tuple], headers: list[str], team_data_type: TEAM_DATA_FILE_TYPES) -> None:
    csv_dir_path = get_team_data_file_path(team_abbreviation, year, team_data_type)

    with open(csv_dir_path, "w") as csv_file:
        csv_writer = csv.writer(csv_file, lineterminator="\n")
        csv_writer.writerow(headers)

        for item in data:
            csv_writer.writerow(item)
    
    print(f"Saved {year} {team_abbreviation} {team_data_type} to {csv_dir_path}")


def save_list_of_all_teams(list_of_teams: list[tuple[str, str]]) -> None:
    with open(f"{RESOURCES_DIR}/all_teams.csv", "w") as csv_file:
        csv_writer = csv.writer(csv_file, lineterminator="\n")
        csv_writer.writerow(["Name","URL","TEAM_ID"])
        for team in list_of_teams:
            csv_writer.writerow(team)


def get_all_teams() -> list[tuple[str, str, str]]:
    result: list[tuple[str, str, str]] = []
    with open(f"{RESOURCES_DIR}/all_teams.csv", "r") as abbreviations:
        csv_reader = csv.reader(abbreviations)
        csv_reader.__next__() # skip headers
        for row in csv_reader:
            result.append(row)

    return result


def read_team_data_file(team_abbreviation: str, year: int, team_data_type: TEAM_DATA_FILE_TYPES) -> list[dict]:
    filename = get_team_data_file_path(team_abbreviation, year, team_data_type)
    return utils.read_csv_as_dict_list(filename)


def save_player_data_file(player_id: str, stat_type: STAT_TYPES, player_data: list[dict]) -> str:
    filename = get_player_data_file_path(player_id, stat_type)
    utils.save_dict_list_to_csv(filename, player_data)
    return filename


def read_player_data_file(player_id: str, stat_type: STAT_TYPES) -> list[dict]:
    filename = get_player_data_file_path(player_id, stat_type)
    return utils.read_csv_as_dict_list(filename)


def get_team_dir_path(team_abbreviation: str) -> str:
    return f"{TEAMS_DIR}/{team_abbreviation}"


def get_team_year_dir_path(team_abbreviation: str, year: int) -> str:
    return f"{TEAMS_DIR}/{team_abbreviation}/{year}"


def get_team_data_file_path(team_abbreviation: str, year: int, team_data_type: TEAM_DATA_FILE_TYPES) -> str:
    return f"{get_team_year_dir_path(team_abbreviation, year)}/{team_abbreviation}_{year}_{team_data_type}.csv"


def get_player_data_file_path(player_id: str, stat_type: STAT_TYPES) -> str:
    return f"{PLAYERS_DIR}/{stat_type}/{player_id}_{stat_type}.csv"


def find_missing_team_data_files(team_abbreviation: str, year: int) -> TEAM_DATA_FILE_TYPES | Literal[""]:
    for data_type in get_args(TEAM_DATA_FILE_TYPES):
        if not team_data_file_exists(team_abbreviation, year, data_type):
            return data_type
    
    return ""


def team_data_file_exists(team_abbreviation: str, year: int, team_data_type: TEAM_DATA_FILE_TYPES) -> bool:
    return os.path.exists(get_team_data_file_path(team_abbreviation, year, team_data_type))


def player_data_file_exists(player_id: str, stat_type: STAT_TYPES) -> bool:
    return os.path.exists(get_player_data_file_path(player_id, stat_type))


if __name__ == "__main__":
    set_up_file_structure()