import os
from typing import Literal, get_args

import utils
from table import Table

PARENT_DIR: str = "data"
TEAMS_DIR: str = f"{PARENT_DIR}/teams"
PLAYERS_DIR: str = f"{PARENT_DIR}/players"
LEAGUE_DIR: str = f"{PARENT_DIR}/league"
RESOURCES_DIR: str = "resources"

PLAYER_STAT_TYPES = Literal["batting", "pitching", "appearances", "fielding", "baserunning", "baserunning_against", "batting_against"]
TEAM_DATA_FILE_TYPES = Literal["roster", "batting", "pitching", "team_info", "common_batting_orders", "schedule"]
LEAGUE_DATA_FILE_TYPES = Literal["batting", "pitching", "fielding", "baserunning", "batting_by_bases", "standings"]

PLAYER_LIST_LOCATIONS_FOR_STATS: dict[PLAYER_STAT_TYPES, TEAM_DATA_FILE_TYPES] = {
    "appearances": "roster",
    "batting": "batting",
    "pitching": "pitching",
    "fielding": "roster",
    "baserunning": "batting",
    "baserunning_against": "pitching",
    "batting_against": "pitching"
}

PLAYER_STAT_EARLIEST_YEARS: dict[PLAYER_STAT_TYPES, int] = {
    "appearances": 0,
    "batting": 0,
    "pitching": 0,
    "fielding": 0,
    "baserunning": 1912,
    "baserunning_against": 1912,
    "batting_against": 1912
}

LEAGUE_STAT_EARLIEST_YEARS: dict[LEAGUE_DATA_FILE_TYPES] = {
    "batting": 0,
    "pitching": 0,
    "fielding": 0,
    "baserunning": 1912,
    "batting_by_bases": 1912,
    "standings": 0
}

PLAYER_STATS_DEPENDENT_ON_BATTING: list[PLAYER_STAT_TYPES] = ["baserunning"]

# Stat tables that Baseball Reference has not switched to the new format
NON_UPDATED_PLAYER_STAT_TABLES: list[PLAYER_STAT_TYPES] = ["baserunning", "batting_against", "baserunning_against"]


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
        if not team_parent_dir_exists(team["TEAM_ID"]):
            return False
    return True


def all_player_folders_exist() -> bool:
    if not os.path.exists(PLAYERS_DIR): return False

    for stat_type in get_player_stat_types():
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

    for stat_type in get_player_stat_types():
        utils.make_dirs(f"{PLAYERS_DIR}/{stat_type}")


def create_all_league_folders():
    utils.make_dirs(LEAGUE_DIR)


def get_all_teams() -> list[dict[Literal["NAME", "URL", "TEAM_ID"], str]]:
    filename = f"{RESOURCES_DIR}/all_teams.csv"
    return utils.read_csv_as_dict_list(filename)


def get_all_saved_teams_from_year(year: int) -> tuple[list[str], list[str]]:
    """ Return main team abbreviations, year-specific abbreviations of all teams that have been locally saved from a year. """
    all_teams = get_all_teams()
    main_abbrs: list[str] = []
    year_abbrs: list[str] = []
    for team in all_teams:
        if team_year_dir_exists(team["TEAM_ID"], year) and len(find_missing_team_data_files(team["TEAM_ID"], year)) == 0:
            info_table = read_team_data_file(team["TEAM_ID"], year, "team_info")
            abbr = info_table.rows[0]["abbreviation"]
            main_abbrs.append(team["TEAM_ID"])
            year_abbrs.append(abbr)
    
    return main_abbrs, year_abbrs


def get_all_real_teams_from_year(year: int) -> list[str]:
    """ Return abbreviations of all teams that existed in the league in a given year. """
    standings_table = read_league_data_file(year, "standings")
    team_abbrs = []
    for row in standings_table:
        team_abbrs.append(row["ID"])
    
    return sorted(team_abbrs)


def get_player_list(team_abbreviation: str, year: int, stat_types: list[PLAYER_STAT_TYPES]) -> list[dict[Literal["ID", "URL", "STAT_TYPES"], str]]:
    found_players: dict[str, tuple[str, list[PLAYER_STAT_TYPES]]] = dict()

    for stat_type in stat_types:
        if year < PLAYER_STAT_EARLIEST_YEARS[stat_type]:
            continue

        team_file_type_to_read_from = get_team_file_type_to_read_from(stat_type)
        team_data = read_team_data_file(team_abbreviation, year, team_file_type_to_read_from)
        for row in team_data:
            if should_load_player_stat_type(row, stat_type):
                if row["ID"] not in found_players.keys():
                    found_players[row["ID"]] = (row["URL"], [stat_type])
                else:
                    found_players[row["ID"]][1].append(stat_type)
    
    result: list[dict[Literal["ID", "URL", "STAT_TYPES"], str]] = []
    for player in found_players.keys():
        url, player_stats = found_players[player]
        result.append({"ID": player, "URL": url, "STAT_TYPES": player_stats})

    return result


def should_load_player_stat_type(player_row_in_team_data: dict[str, str], stat_type: PLAYER_STAT_TYPES) -> bool:
    if (stat_type == "baserunning") and (int(player_row_in_team_data.get("b_h", 0)) == 0):
        return False

    elif (stat_type == "fielding") and (int(player_row_in_team_data.get("games_defense", 0)) == 0):
        return False
    
    return True


def is_player_file_up_to_date(player_id: str, stat_type: PLAYER_STAT_TYPES, year: int) -> bool:
    """ Return true if the data for the given year exists within the specified player data file. If not (or if the player file doesn't exist), return false. """
    if not player_data_file_exists(player_id, stat_type):
        return False
    
    player_data = read_player_data_file(player_id, stat_type)

    if stat_type in NON_UPDATED_PLAYER_STAT_TABLES:
        search_attributes = {"year_ID": str(year)}
    else:
        search_attributes = {"year_id": str(year)}
    search_results = player_data.search_rows(search_attributes)

    return len(search_results) > 0


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


def create_league_year_dir(year: int) -> None:
    dir_path: str = get_league_year_dir_path(year)
    utils.make_dirs(dir_path)


def save_league_data_file(year: int, stat_type: LEAGUE_DATA_FILE_TYPES, league_data: Table) -> str:
    filename = get_league_data_file_path(stat_type, year)
    utils.save_table_to_csv(filename, league_data)
    print(f"Saved {year} League {stat_type} table to {filename}")
    return filename


def read_league_data_file(year: int, stat_type: LEAGUE_DATA_FILE_TYPES) -> Table:
    filename = get_league_data_file_path(stat_type, year)
    return utils.read_csv_as_table(filename)


def get_team_dir_path(team_abbreviation: str) -> str:
    return f"{TEAMS_DIR}/{team_abbreviation}"


def get_team_year_dir_path(team_abbreviation: str, year: int) -> str:
    return f"{get_team_dir_path(team_abbreviation)}/{year}"


def get_team_data_file_path(team_abbreviation: str, year: int, team_data_type: TEAM_DATA_FILE_TYPES) -> str:
    return f"{get_team_year_dir_path(team_abbreviation, year)}/{team_abbreviation}_{year}_{team_data_type}.csv"


def get_player_data_file_path(player_id: str, stat_type: PLAYER_STAT_TYPES) -> str:
    return f"{PLAYERS_DIR}/{stat_type}/{player_id}_{stat_type}.csv"


def get_league_year_dir_path(year: int) -> str:
    return f"{LEAGUE_DIR}/{year}"


def get_league_data_file_path(stat_type: LEAGUE_DATA_FILE_TYPES, year: int) -> str:
    return f"{get_league_year_dir_path(year)}/{year}_league_{stat_type}.csv"


def find_missing_team_data_files(team_abbreviation: str, year: int) -> list[TEAM_DATA_FILE_TYPES]:
    missing_data_types = []
    for data_type in get_team_data_file_types():
        if not team_data_file_exists(team_abbreviation, year, data_type):
            missing_data_types.append(data_type)
    
    return missing_data_types


def team_parent_dir_exists(team_abbreviation: str) -> bool:
    return os.path.exists(get_team_dir_path(team_abbreviation))


def team_year_dir_exists(team_abbreviation: str, year: int) -> bool:
    return os.path.exists(get_team_year_dir_path(team_abbreviation, year))


def team_data_file_exists(team_abbreviation: str, year: int, team_data_type: TEAM_DATA_FILE_TYPES) -> bool:
    return os.path.exists(get_team_data_file_path(team_abbreviation, year, team_data_type))


def player_data_file_exists(player_id: str, stat_type: PLAYER_STAT_TYPES) -> bool:
    return os.path.exists(get_player_data_file_path(player_id, stat_type))


def league_year_dir_exists(year: int) -> bool:
    return os.path.exists(get_league_year_dir_path(year))


def league_data_file_exists(stat_type: LEAGUE_DATA_FILE_TYPES, year: int) -> bool:
    return os.path.exists(get_league_data_file_path(stat_type, year))


def get_team_file_type_to_read_from(player_stat_type: PLAYER_STAT_TYPES) -> TEAM_DATA_FILE_TYPES:
    """
    Given the stat type, returns what type of team file should be read to retrieve the correct players.
    For example, if our player_stat_type is 'pitching', then this will return the team data file type where the list of a team's pitchers are located.
    """

    team_file_type_to_read_from = PLAYER_LIST_LOCATIONS_FOR_STATS[player_stat_type]
    return team_file_type_to_read_from


def get_player_stat_types() -> tuple[PLAYER_STAT_TYPES, ...]:
    return get_args(PLAYER_STAT_TYPES)


def get_team_data_file_types() -> tuple[TEAM_DATA_FILE_TYPES, ...]:
    return get_args(TEAM_DATA_FILE_TYPES)


def get_league_data_file_types() -> tuple[LEAGUE_DATA_FILE_TYPES, ...]:
    return get_args(LEAGUE_DATA_FILE_TYPES)


if __name__ == "__main__":
    set_up_file_structure()