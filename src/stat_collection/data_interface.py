import os, sys, time, csv

import utils

TEAMS_DIR: str = "data/teams"
PLAYERS_DIR: str = "data/players"
RESOURCES_DIR: str = "resources"

def set_up_file_structure():
    utils.make_dirs("data")
    utils.make_dirs(PLAYERS_DIR)

    create_all_team_folders()


def create_all_team_folders():
    utils.make_dirs(TEAMS_DIR)

    for team_name, team_url, team_abbrev in get_all_teams():
        create_team_folder(team_abbrev)


def create_team_folder(team_abbreviation: str) -> None:
    dir_path: str = get_team_dir_path(team_abbreviation)
    utils.make_dirs(dir_path)


def create_team_year_folder(team_abbreviation: str, year: str) -> None:
    dir_path: str = get_team_year_dir_path(team_abbreviation, year)
    utils.make_dirs(dir_path)


def save_team_roster_file(team_abbreviation: str, year: str, roster: list):
    file_dir = get_team_year_dir_path(team_abbreviation, year)
    csv_dir_path = f"{file_dir}/{team_abbreviation}_{year}_roster.csv"

    with open(csv_dir_path, "w") as csv_file:
        csv_writer = csv.writer(csv_file, lineterminator="\n")
        csv_writer.writerow(["NAME", "ID", "POS", "URL"])

        for player in roster:
            csv_writer.writerow(player)


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


def get_team_dir_path(team_abbreviation: str) -> str:
    return f"{TEAMS_DIR}/{team_abbreviation}"


def get_team_year_dir_path(team_abbreviation: str, year: str) -> str:
    return f"{TEAMS_DIR}/{team_abbreviation}/{year}"


def get_player_dir_path(player_id: str) -> str:
    return f"{PLAYERS_DIR}/{player_id}"


if __name__ == "__main__":
    set_up_file_structure()