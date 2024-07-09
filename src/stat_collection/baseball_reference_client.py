from bs4 import BeautifulSoup
import urllib.request
import time
import random

import data_interface as DI
import utils

BASE_URL: str = 'http://www.baseball-reference.com'
DEFAULT_EVADE_SLEEP_MIN: int = 4 #seconds
DEFAULT_EVADE_SLEEP_MAX: int = 7

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
        roster_table = soup.find("table", {"id": "team_batting"}).find("tbody")
        table_rows = roster_table.findAll("tr")
        for row in table_rows:
            player_info = row.find("td", {"data-stat": "player"})
            if player_info != None:
                name = player_info.get("csk").replace(",", ";")
                player_id = player_info.get("data-append-csv")
                player_url = BASE_URL + player_info.find("a").get("href")

                pos = row.find("td", {"data-stat": "pos"}).find(string=True)
                roster.append((name, player_id, pos, player_url))

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

if __name__ == "__main__":
    client = Scraping_Client()
    teams = DI.get_all_teams()
    year = str(1915)

    for team_name, team_url, team_abbrev in teams:
        current_year_page = client.scrape_team_page_for_roster_url(team_url, year)
        if current_year_page != None:
            DI.create_team_year_folder(team_abbrev, year)
            roster = client.scrape_team_roster_page(current_year_page)
            DI.save_team_roster_file(team_abbrev, year, roster)