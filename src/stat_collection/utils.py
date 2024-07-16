import os
from datetime import datetime
from bs4 import BeautifulSoup, Comment

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