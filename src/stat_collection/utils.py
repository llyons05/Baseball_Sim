import os
from datetime import datetime
import csv
import pandas
import tabulate

from table import Table

BASE_URL = 'http://www.baseball-reference.com'


def get_current_year() -> int:
    return datetime.now().year


def make_dirs(dir_path: str) -> None:
    if not os.path.exists(dir_path):
        os.makedirs(dir_path, exist_ok=True)


def get_abbreviation_from_overall_team_page_url(url: str) -> str:
    return url.removesuffix("/").split("/")[-1]


def get_abbreviation_from_specific_team_page_url(url: str) -> str:
    return url.removesuffix("/").removeprefix(BASE_URL + "/teams/").split("/")[0]


def get_team_roster_url(team_abbreviation: str, year: int) -> str:
    return BASE_URL + "/teams/" + team_abbreviation + "/" + str(year) + ".shtml"


def get_team_batting_order_url(team_roster_url: str) -> str:
    return team_roster_url.replace(".shtml", "-batting-orders.shtml")


def get_team_schedule_url(team_roster_url: str) -> str:
    return team_roster_url.replace(".shtml", "-schedule-scores.shtml")


def get_team_id_from_url(team_page_url: str) -> str:
    return team_page_url.split("/")[-2]


def get_player_id_from_url(player_page_url: str) -> str:
    return player_page_url.split("/")[-1].split(".shtml")[0]


def get_player_batting_page_url(base_player_page_url: str) -> str:
    return base_player_page_url.replace(".shtml", "-bat.shtml")


def get_player_pitching_page_url(base_player_page_url: str) -> str:
    return base_player_page_url.replace(".shtml", "-pitch.shtml")


def get_base_league_year_url(year: int) -> str:
    return f"{BASE_URL}/leagues/majors/{year}.shtml"


def get_league_standings_url(base_league_year_url: str) -> str: 
    return base_league_year_url.replace(".shtml", "-standings.shtml")


def save_table_to_csv(filename: str, table: Table) -> None:
    headers = table.headers
    dict_list = table.rows

    save_dict_list_to_csv(filename, dict_list, headers)


def save_dict_list_to_csv(filename: str, dict_list: list[dict], headers: list = []) -> None:
    csv_headers = []
    if not headers:
        csv_headers = get_dict_list_headers(dict_list)
    
    else:
        csv_headers = headers

    with open(filename, "w", encoding='utf-8') as csv_file:
        csv_writer = csv.writer(csv_file, lineterminator="\n")
        csv_writer.writerow(csv_headers)

        for dictionary in dict_list:
            row_of_data: list = []
            for key in csv_headers:
                key_index = csv_headers.index(key)
                row_of_data.insert(key_index, dictionary.get(key))
            csv_writer.writerow(row_of_data)


def read_csv_as_dict_list(filename: str, headers_to_get: list[str] = []) -> list[dict]:
    result = []

    with open(filename, "r", encoding='utf-8') as csv_file:
        csv_reader = csv.reader(csv_file)
        headers = csv_reader.__next__()

        for row in csv_reader:
            formatted_row = dict()

            column_in_row = 0
            for value in row:
                header = headers[column_in_row]
                if headers_to_get and header not in headers_to_get:
                    column_in_row += 1
                    continue

                formatted_row[header] = value
                column_in_row += 1
            
            result.append(formatted_row)
    
    return result


def get_dict_list_headers(dict_list: list[dict]) -> list:
    headers = []
    for dictionary in dict_list:
        for key in dictionary.keys():
            if key not in headers:
                headers.append(key)
    
    return headers


def read_csv_as_table(filename: str) -> Table:
    result = []

    with open(filename, "r", encoding='utf-8') as csv_file:
        csv_reader = csv.reader(csv_file)
        headers = csv_reader.__next__()
        for row in csv_reader:
            formatted_row = dict()

            column_in_row = 0
            for value in row:
                header = headers[column_in_row]
                formatted_row[header] = value
                column_in_row += 1
            
            result.append(formatted_row)
    
    return Table(headers, result)


def print_csv(csv_filename: str, headers: list[str] = []) -> None:
    csv_data = read_csv_as_dict_list(csv_filename, headers)
    dataframe = pandas.DataFrame(csv_data)
    data_headers = get_dict_list_headers(csv_data)
    print(tabulate.tabulate(dataframe, data_headers, "grid", showindex=False))