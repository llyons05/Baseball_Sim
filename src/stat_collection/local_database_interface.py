import os, sys, time, csv
from typing import Literal, get_args

import utils
from table import Table

PARENT_DIR: str = "data"
TEAMS_DIR: str = f"{PARENT_DIR}/teams"
PLAYERS_DIR: str = f"{PARENT_DIR}/players"
LEAGUE_DIR: str = f"{PARENT_DIR}/league"
RESOURCES_DIR: str = "resources"

PLAYER_STAT_TYPES = Literal["batting", "pitching", "appearances", "baserunning"]
TEAM_DATA_FILE_TYPES = Literal["roster", "batting", "pitching", "team_info", "common_batting_orders"]

PLAYER_LIST_LOCATIONS_FOR_STATS: dict[PLAYER_STAT_TYPES, TEAM_DATA_FILE_TYPES] = {
    "appearances": "roster",
    "batting": "batting",
    "pitching": "pitching",
    "baserunning": "batting"
}


def set_up_file_structure():
    utils.make_dirs(PARENT_DIR)
    create_all_team_folders()
    create_all_player_folders()
    create_all_league_folders()


def is_file_structure_set_up() -> bool:
    return all_team_folders_exist() and all_player_folders_exist() and all_league_folders_exist()


def all_team_folders_exist() -> bool:
    if not os.path.exists(TEAMS_DIR): return False

    for team in get_all_teams():
        if not os.path.exists(get_team_dir_path(team["TEAM_ID"])):
            return False
    return True


def all_player_folders_exist() -> bool:
    if not os.path.exists(PLAYERS_DIR): return False

    for stat_type in get_args(PLAYER_STAT_TYPES):
        if not os.path.exists(f"{PLAYERS_DIR}/{stat_type}"):
            return False
    return True


def all_league_folders_exist() -> bool:
    return os.path.exists(LEAGUE_DIR)


def create_all_team_folders():
    utils.make_dirs(TEAMS_DIR)

    for team in get_all_teams():
        create_team_folder(team["TEAM_ID"])


def create_all_player_folders():
    utils.make_dirs(PLAYERS_DIR)

    for stat_type in get_args(PLAYER_STAT_TYPES):
        utils.make_dirs(f"{PLAYERS_DIR}/{stat_type}")


def create_all_league_folders():
    utils.make_dirs(LEAGUE_DIR)


def get_all_teams() -> list[dict[Literal["NAME", "URL", "TEAM_ID"], str]]:
    filename = f"{RESOURCES_DIR}/all_teams.csv"
    return utils.read_csv_as_dict_list(filename)


def create_team_folder(team_abbreviation: str) -> None:
    dir_path: str = get_team_dir_path(team_abbreviation)
    utils.make_dirs(dir_path)


def create_team_year_folder(team_abbreviation: str, year: int) -> None:
    dir_path: str = get_team_year_dir_path(team_abbreviation, year)
    utils.make_dirs(dir_path)


def save_team_data_file(team_abbreviation: str, year: int, data: Table, team_data_type: TEAM_DATA_FILE_TYPES) -> None:
    csv_dir_path = get_team_data_file_path(team_abbreviation, year, team_data_type)
    utils.save_table_to_csv(csv_dir_path, data)
    print(f"Saved {year} {team_abbreviation} {team_data_type} to {csv_dir_path}")


def read_team_data_file(team_abbreviation: str, year: int, team_data_type: TEAM_DATA_FILE_TYPES) -> Table:
    filename = get_team_data_file_path(team_abbreviation, year, team_data_type)
    return utils.read_csv_as_table(filename)


def save_player_data_file(player_id: str, stat_type: PLAYER_STAT_TYPES, player_data: Table) -> str:
    filename = get_player_data_file_path(player_id, stat_type)
    utils.save_table_to_csv(filename, player_data)
    return filename


def read_player_data_file(player_id: str, stat_type: PLAYER_STAT_TYPES) -> Table:
    filename = get_player_data_file_path(player_id, stat_type)
    return utils.read_csv_as_table(filename)


def save_league_data_file(stat_type: Literal["batting", "pitching"], league_data: Table) -> str:
    filename = get_league_data_file_path(stat_type)
    utils.save_table_to_csv(filename, league_data)
    print(f"Saved League {stat_type} table to {filename}")
    return filename


def read_league_data_file(stat_type: Literal["batting", "pitching"]) -> Table:
    filename = get_league_data_file_path(stat_type)
    return utils.read_csv_as_table(filename)


def get_team_dir_path(team_abbreviation: str) -> str:
    return f"{TEAMS_DIR}/{team_abbreviation}"


def get_team_year_dir_path(team_abbreviation: str, year: int) -> str:
    return f"{TEAMS_DIR}/{team_abbreviation}/{year}"


def get_team_data_file_path(team_abbreviation: str, year: int, team_data_type: TEAM_DATA_FILE_TYPES) -> str:
    return f"{get_team_year_dir_path(team_abbreviation, year)}/{team_abbreviation}_{year}_{team_data_type}.csv"


def get_player_data_file_path(player_id: str, stat_type: PLAYER_STAT_TYPES) -> str:
    return f"{PLAYERS_DIR}/{stat_type}/{player_id}_{stat_type}.csv"


def get_league_data_file_path(stat_type: Literal["batting", "pitching"]) -> str:
    return f"{LEAGUE_DIR}/league_{stat_type}_avg.csv"


def find_missing_team_data_files(team_abbreviation: str, year: int) -> TEAM_DATA_FILE_TYPES | Literal[""]:
    for data_type in get_args(TEAM_DATA_FILE_TYPES):
        if not team_data_file_exists(team_abbreviation, year, data_type):
            return data_type
    
    return ""


def team_data_file_exists(team_abbreviation: str, year: int, team_data_type: TEAM_DATA_FILE_TYPES) -> bool:
    return os.path.exists(get_team_data_file_path(team_abbreviation, year, team_data_type))


def player_data_file_exists(player_id: str, stat_type: PLAYER_STAT_TYPES) -> bool:
    return os.path.exists(get_player_data_file_path(player_id, stat_type))


def league_data_file_exists(stat_type: Literal["batting", "pitching"]) -> bool:
    return os.path.exists(get_league_data_file_path(stat_type))


def get_team_file_type_to_read_from(player_stat_type: PLAYER_STAT_TYPES) -> TEAM_DATA_FILE_TYPES:
    """
    Given the stat type, returns what type of team file should be read to retrieve the correct players.
    For example, if our player_stat_type is 'pitching', then this will return the team data file type where the pitchers are located.
    """

    team_file_type_to_read_from = PLAYER_LIST_LOCATIONS_FOR_STATS[player_stat_type]
    
    return team_file_type_to_read_from


if __name__ == "__main__":
    set_up_file_structure()