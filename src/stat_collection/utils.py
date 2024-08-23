import os
from datetime import datetime
from bs4 import BeautifulSoup, Comment
import csv

BASE_URL = 'http://www.baseball-reference.com'

def get_current_year() -> str:
    return str(datetime.now().year)

def make_dirs(dir_path: str) -> None:
    if not os.path.exists(dir_path):
        os.makedirs(dir_path, exist_ok=True)

def get_abbreviation_from_team_page_url(url: str) -> str:
    return url.removesuffix("/").split("/")[-1]

def get_team_roster_url(team_abbreviation: str, year: str) -> str:
    return BASE_URL + "/teams/" + team_abbreviation + "/" + str(year) + ".shtml"

def get_player_id_from_url(player_page_url: str) -> str:
    return player_page_url.split("/")[-1].split(".")[0]

def get_player_batting_page_url(base_player_page_url: str) -> str:
    return base_player_page_url.replace(".shtml", "-bat.shtml")

def get_player_pitching_page_url(base_player_page_url: str) -> str:
    return base_player_page_url.replace(".shtml", "-pitch.shtml")

def save_dict_list_to_csv(filename: str, dict_list: list[dict], headers: list = []) -> None:
    csv_headers = []
    if not headers:
        for dictionary in dict_list:
            for key in dictionary.keys():
                if key not in csv_headers:
                    csv_headers.append(key)
    
    else:
        csv_headers = headers

    with open(filename, "w") as csv_file:
        csv_writer = csv.writer(csv_file, lineterminator="\n")
        csv_writer.writerow(csv_headers)

        for dictionary in dict_list:
            row_of_data: list = []
            for key in csv_headers:
                key_index = csv_headers.index(key)
                row_of_data.insert(key_index, dictionary.get(key))
            
            csv_writer.writerow(row_of_data)

def read_csv_as_dict_list(filename: str) -> list[dict]:
    result = []

    with open(filename, "r") as csv_file:
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
    
    return result