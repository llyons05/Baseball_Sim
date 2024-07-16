from bs4 import BeautifulSoup, Comment
import urllib.request
import time
import random
import re

import local_database_interface as DI
import utils
from table_parser import Table_Parser

BASE_URL = utils.BASE_URL
DEFAULT_EVADE_SLEEP_MIN: int = 4 #seconds
DEFAULT_EVADE_SLEEP_MAX: int = 7

default_stats: list = ["PA", "H", "2B", "3B", "HR", "SB", "CS", "BB", "SO", "Pit", "Str%"]
class Scraping_Client:

    def __init__(self):
        self.team_abbreviations: list[str] = DI.get_all_teams()


    def evade(self) -> None:
        time.sleep(random.randint(DEFAULT_EVADE_SLEEP_MIN, DEFAULT_EVADE_SLEEP_MAX))


    def scrape_page_html(self, url: str):
        self.evade()

        try:
            with urllib.request.urlopen(url) as response:
                html = response.read()
                return html
        except:
            return None


    def scrape_team_roster_page(self, team_roster_page_url: str) -> list[str]:
        roster: list[tuple] = []
        response = self.scrape_page_html(team_roster_page_url)
        if response is None:
            return None

        soup = BeautifulSoup(response, "html.parser")
        row_filters = [
            {
                'value_name': 'class',
                'filtered_values': ['thead']
            }
        ]
        roster_table = utils.parse_data_table(soup, "team_batting", {'player': ['csk', 'data-append-csv']}, row_filters)
        for row in roster_table['data']:
            player_info: dict = row.get('player', dict())
            name = player_info.get('csk', '').replace(",", ";")
            id = player_info.get('data-append-csv', '')
            url = BASE_URL + player_info.get('href')
            pos = row.get('pos', dict()).get('text', '')
            roster.append((name, id, pos, url))

        return roster


    def scrape_all_teams_list(self) -> list[tuple[str, str]]:
        team_data_list: list[tuple[str, str]] = []
        teams_page_url: str = f"{BASE_URL}/teams/"
        response = self.scrape_page_html(teams_page_url)

        if response is None:
            return None

        soup = BeautifulSoup(response, "html.parser")

        teams_table = soup.find("table", {"id": "teams_active"})
        team_cells = teams_table.findAll("td", {"data-stat": "franchise_name"})

        for cell in team_cells:
            team_name = cell.find(string=True)
            team_url = BASE_URL + cell.find("a").get("href")
            team_abbrev = utils.get_abbreviation_from_team_page_url(team_url)
            team_data_list.append((team_name, team_url, team_abbrev))

        return team_data_list


    def scrape_team_page_for_roster_url(self, team_page_url: str, year: str) -> str | None:
        response = self.scrape_page_html(team_page_url)
        if response is None:
            return None

        soup = BeautifulSoup(response, "html.parser")
        franchise_years_table = soup.find("table", {"id": "franchise_years"})
        table_rows = franchise_years_table.findAll("th", {"data-stat": "year_ID"})

        for row in table_rows:
            row_year = row.find(string=True)
            if row_year == year:
                return BASE_URL + row.find("a").get("href")
        
        return None


    def scrape_player_batting(self, player_page_url: str) -> list[dict]:
        if "-bat.shtml" not in player_page_url:
            player_page_url = utils.get_player_batting_page_url(player_page_url)

        response = self.scrape_page_html(player_page_url)
        if response is None:
            return None
        
        soup = BeautifulSoup(response, "html.parser")
        table_parser = Table_Parser(soup, "batting_standard", "all_batting_standard")
        row_filters = [
            {
                'value_name': 'class',
                'filtered_values': ['hidden', 'spacer']
            }
        ]

        final_stats = table_parser.parse(row_filters=row_filters)

        print(final_stats)
        return final_stats

if __name__ == "__main__":
    client = Scraping_Client()
    # https://www.reddit.com/r/learnpython/comments/167qy0w/beautiful_soup_help_wanted/
    # https://stackoverflow.com/questions/42753546/parsing-html-in-with-beautifulsoup-fails-to-find-a-table
    player_link = "http://www.baseball-reference.com/players/s/scherma01.shtml"
    client.scrape_player_batting(player_link)