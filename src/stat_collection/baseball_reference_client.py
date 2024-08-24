from bs4 import BeautifulSoup
import urllib.request
import time
import random

import utils
import local_database_interface as DI
from table_parser import Table_Parser, NoTableFoundException

BASE_URL = utils.BASE_URL
DEFAULT_EVADE_SLEEP_MIN: int = 4 # seconds
DEFAULT_EVADE_SLEEP_MAX: int = 7

EMPTY_TABLE = {"data": [], "headers": []}

class Scraping_Client:

    def __init__(self):
        pass


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

        table_parser = Table_Parser(response, "team_batting", "all_team_batting")

        row_filters = [
            {
                'value_name': 'class',
                'filtered_values': ['thead']
            }
        ]

        cell_specific_data = {
            'player': [
                'csk',
                'data-append-csv'
            ]
        }

        roster_table = table_parser.parse(cell_specific_data=cell_specific_data, row_filters=row_filters)

        for row in roster_table['data']:
            player_info: dict = row.get('player', dict())

            lastname, firstname = player_info.get('csk', '').split(",")
            id = player_info.get('data-append-csv', '')
            url = BASE_URL + player_info.get('href')
            pos = row.get('pos', dict()).get('text', '')

            roster.append((firstname, lastname, id, pos, url))

        return roster


    def scrape_all_teams_list(self) -> list[tuple[str, str, str]]:
        team_data_list: list[tuple[str, str]] = []
        teams_page_url: str = f"{BASE_URL}/teams/"
        response = self.scrape_page_html(teams_page_url)

        if response is None:
            return None

        table_parser = Table_Parser(response, "teams_active", "all_teams_active")

        row_filters = [
            {
                'value_name': 'class',
                'filtered_values': ['hidden', 'spacer', 'thead']
            },
            {
                'interior_tags': ['span'],
                'value_name': 'class',
                'filtered_values': ["moved_names", "alternate_names"]
            }

        ]

        teams_table = table_parser.parse(row_filters=row_filters)

        for row in teams_table['data']:
            team_name = row['franchise_name']['text']
            team_url = BASE_URL + row['franchise_name']['href']
            team_abbrev = utils.get_abbreviation_from_team_page_url(team_url)
            team_data_list.append((team_name, team_url, team_abbrev))

        return team_data_list


    def scrape_team_page_for_roster_url(self, team_page_url: str, year: int) -> str | None:
        response = self.scrape_page_html(team_page_url)
        if response is None:
            return None

        table_parser = Table_Parser(response, "franchise_years", "all_franchise_years")
        franchise_years_table = table_parser.parse()

        for row in franchise_years_table['data']:
            row_year = row['year_ID']
            if row_year['text'] == str(year):
                return BASE_URL + row_year['href']

        return None


    def scrape_player_stats(self, base_player_page_url: str, stat_type: DI.STAT_TYPES) -> list[dict]:
        table_parser = None
        if stat_type == "batting":
            table_parser = self.try_scraping_batting_tables(base_player_page_url)
        elif stat_type == "pitching":
            table_parser = self.try_scraping_pitching_tables(base_player_page_url)

        if table_parser is None:
            return self.format_player_data_table(EMPTY_TABLE)

        row_filters = [
            {
                'value_name': 'class',
                'filtered_values': ['hidden', 'spacer', 'nonroster_table']
            }
        ]

        player_data_table = table_parser.parse(row_filters=row_filters)

        return self.format_player_data_table(player_data_table)


    def try_scraping_batting_tables(self, base_player_page_url: str) -> Table_Parser | None:
        player_batting_page_url = utils.get_player_batting_page_url(base_player_page_url)
        table_parser = self.scrape_table_from_player_page(player_batting_page_url, "batting_standard", "all_batting_standard")

        if table_parser is None:
            table_parser = self.scrape_table_from_player_page(base_player_page_url, "batting_standard", "all_batting_standard")
        
        return table_parser


    def try_scraping_pitching_tables(self, base_player_page_url: str) -> Table_Parser | None:
        player_pitching_page_url = utils.get_player_pitching_page_url(base_player_page_url)
        table_parser = self.scrape_table_from_player_page(player_pitching_page_url, "pitching_standard", "all_pitching_standard")

        if table_parser is None:
            table_parser = self.scrape_table_from_player_page(base_player_page_url, "pitching_standard", "all_pitching_standard")

        return table_parser


    def scrape_table_from_player_page(self, player_page_url: str, table_id: str, table_parent_div_id: str) -> Table_Parser | None:
        response = self.scrape_page_html(player_page_url)
        if response is None:
            return None

        try:
            table_parser = Table_Parser(response, table_id, table_parent_div_id)

        except NoTableFoundException as e:
            return None
        
        return table_parser


    def format_player_data_table(self, player_data_table: dict) -> list[dict]:
        headers: list[str] = player_data_table["headers"]
        data: list[dict[str, dict]] = player_data_table["data"]

        result = []

        for year_of_data in data:
            formatted_year_of_data: dict = {}
            for header_name in headers:
                formatted_year_of_data[header_name] = year_of_data.get(header_name, {}).get("text")
            result.append(formatted_year_of_data)

        result.sort(key=lambda d: int(d.get("year_ID", 0)))
        return result


if __name__ == "__main__":
    client = Scraping_Client()
    player_link = "http://www.baseball-reference.com/players/s/stantmi03.shtml"
    stats = client.scrape_player_stats(player_link, "batting")

    utils.save_dict_list_to_csv("test_player_batting.csv", stats)