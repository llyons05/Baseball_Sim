from bs4 import BeautifulSoup
import urllib.request
import time
import random
from typing import Literal
from copy import deepcopy

import utils
import local_database_interface as DI
from table_parser import Table_Parser, NoTableFoundException
import table_parsing_types as Extra_Types
from table import Table, EMPTY_TABLE

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


    def scrape_team_data(self, team_roster_page_url: str) -> dict[DI.TEAM_DATA_FILE_TYPES, Table]:
        response = self.scrape_page_html(team_roster_page_url)

        roster_data = self.parse_team_roster_table(response)
        batting_data = self.parse_team_batting_table(response)
        pitching_data = self.parse_team_pitching_table(response)
        basic_team_info_data = self.parse_team_info_table(response, team_roster_page_url)
        batting_order_data = self.get_team_common_batting_orders_table(team_roster_page_url)

        result: dict[DI.TEAM_DATA_FILE_TYPES, Table] = dict()
        result["roster"] = roster_data
        result["batting"] = batting_data
        result["pitching"] = pitching_data
        result["team_info"] = basic_team_info_data
        result["common_batting_orders"] = batting_order_data

        return result


    def parse_team_roster_table(self, main_team_roster_page_html) -> Table:
        parser = Table_Parser(main_team_roster_page_html, self.get_default_table_location("appearances"), self.get_default_wrapper_div_location("all_appearances"))

        extra_row_vals = deepcopy(DEFAULT_PLAYER_TABLE_EXTRA_ROW_VALS)
        for val in extra_row_vals[:3]:
            val["location"]["tag_navigation_path"][0]["tag_name"] = "th"

        roster = parser.parse(extra_row_vals, DEFAULT_PLAYER_TABLE_ROW_FILTERS, ["player", "ranker"], DEFAULT_FORBIDDEN_CHARS)
        return roster


    def parse_team_pitching_table(self, main_team_roster_page_html) -> Table:
        pitchers = self.parse_default_player_list_table(main_team_roster_page_html, "team_pitching", "all_team_pitching")
        for row in pitchers.rows:
            if not row["pos"]:
                row["pos"] = "P"
    
        return pitchers


    def parse_team_batting_table(self, main_team_roster_page_html) -> Table:
        batters = self.parse_default_player_list_table(main_team_roster_page_html, "team_batting", "all_team_batting")
        return batters


    def parse_default_player_list_table(self, main_team_roster_page_html, table_id: str, table_parent_div_id: str) -> Table:
        parser = Table_Parser(main_team_roster_page_html, self.get_default_table_location(table_id), self.get_default_wrapper_div_location(table_parent_div_id))
        table_data = parser.parse(DEFAULT_PLAYER_TABLE_EXTRA_ROW_VALS, DEFAULT_PLAYER_TABLE_ROW_FILTERS, ["player", "ranker"], DEFAULT_FORBIDDEN_CHARS)
        return table_data


    def parse_team_info_table(self, main_team_roster_page_html, team_year_page_url: str) -> Table:
        result = Table()
        team_abbreviation = utils.get_abbreviation_from_specific_team_page_url(team_year_page_url)

        result.add_row({"abbreviation": team_abbreviation}, True)

        return result


    def get_team_common_batting_orders_table(self, team_roster_page_url: str) -> Table:
        result = Table()
        url = utils.get_team_batting_order_url(team_roster_page_url)
        response = self.scrape_page_html(url)

        wrapper_div_location = [{"tag_name": "div", "attributes": {"id": "all_common_orders"}}]
        table_location = [wrapper_div_location[0], {"tag_name": "table", "attributes": {"class": "stats_table"}}]
        parser = Table_Parser(response, table_location, wrapper_div_location)

        value_locations: list[Extra_Types.HTML_TAG_VALUE_LOCATION] = []
        for x in range(1, 10):
            value_locations.append({
                "attribute_name": "data-entry-id",
                "tag_navigation_path": [
                    {"tag_name": "li", "attributes": {"value": str(x)}}, {"tag_name": "a"}
                ]
            })
        
        columns: list[BeautifulSoup] = parser.table.find_all("td")
        for column in columns:
            column_data = dict()
            column_data["num_games"] = column.find("strong").find(string=True).split()[0]
            for location in value_locations:
                pos_in_batting_order = location["tag_navigation_path"][0]["attributes"]["value"]
                column_data[pos_in_batting_order] = parser.get_value_from_tag(column, location)
            
            result.add_row(column_data, True)

        return result



    def scrape_team_page_for_roster_url(self, team_page_url: str, year: int) -> str | None:
        response = self.scrape_page_html(team_page_url)
        if response is None:
            return None

        extra_columns: list[Extra_Types.EXTRA_ROW_VALUE] = [
            {
                "name": "URL",
                "location": {
                    "tag_navigation_path": [{"tag_name": "th"}, {"tag_name": "a"}],
                    "attribute_name": "href"
                }
            }
        ]
        table_parser = Table_Parser(response, self.get_default_table_location("franchise_years"), self.get_default_wrapper_div_location("all_franchise_years"))
        franchise_years_table = table_parser.parse(extra_columns)

        search_results = franchise_years_table.search_rows({"year_ID": str(year)})
        if (len(search_results) > 0):
            return BASE_URL + search_results[0]["URL"]

        return None


    def scrape_player_stats(self, base_player_page_url: str, stat_type: DI.PLAYER_STAT_TYPES) -> Table:
        table_parser = None
        if stat_type == "batting":
            table_parser = self.try_scraping_batting_tables(base_player_page_url)
        elif stat_type == "pitching":
            table_parser = self.try_scraping_pitching_tables(base_player_page_url)
        elif stat_type == "appearances":
            table_parser = self.try_scraping_appearance_tables(base_player_page_url)

        if table_parser is None:
            return EMPTY_TABLE

        row_filters: list[Extra_Types.TABLE_ROW_FILTER] = [
            {
                "value_location": {"attribute_name": "class"},
                "filtered_values": ["hidden", "spacer", "nonroster_table"]
            }
        ]

        player_data_table = table_parser.parse(row_filters=row_filters)
        player_data_table.sort(key=lambda d: int(d.get("year_ID", 0)))
        return player_data_table


    def try_scraping_batting_tables(self, base_player_page_url: str) -> Table_Parser | None:
        player_batting_page_url = utils.get_player_batting_page_url(base_player_page_url)
        table_location = self.get_default_table_location("batting_standard")
        wrapper_div_location = self.get_default_wrapper_div_location("all_batting_standard")

        table_parser = self.scrape_table_from_player_page(player_batting_page_url, table_location, wrapper_div_location)

        if table_parser is None:
            table_parser = self.scrape_table_from_player_page(base_player_page_url, table_location, wrapper_div_location)
        
        return table_parser


    def try_scraping_pitching_tables(self, base_player_page_url: str) -> Table_Parser | None:
        player_pitching_page_url = utils.get_player_pitching_page_url(base_player_page_url)
        table_location = self.get_default_table_location("pitching_standard")
        wrapper_div_location = self.get_default_wrapper_div_location("all_pitching_standard")

        table_parser = self.scrape_table_from_player_page(player_pitching_page_url, table_location, wrapper_div_location)

        if table_parser is None:
            table_parser = self.scrape_table_from_player_page(base_player_page_url, table_location, wrapper_div_location)

        return table_parser


    def try_scraping_appearance_tables(self, base_player_page_url: str) -> Table_Parser | None:
        table_location = self.get_default_table_location("appearances")
        wrapper_div_location = self.get_default_wrapper_div_location("all_appearances")

        table_parser = self.scrape_table_from_player_page(base_player_page_url, table_location, wrapper_div_location)

        return table_parser


    def scrape_table_from_player_page(self, player_page_url: str,
                                      table_location: Extra_Types.HTML_TAG_NAVIGATION_PATH,
                                      table_wrapper_div_location: Extra_Types.HTML_TAG_NAVIGATION_PATH) -> Table_Parser | None:
        response = self.scrape_page_html(player_page_url)
        if response is None:
            return None

        try:
            table_parser = Table_Parser(response, table_location, table_wrapper_div_location)

        except NoTableFoundException as e:
            print(f"NO TABLE FOUND ON PAGE {player_page_url}")
            return None
        
        return table_parser


    def get_default_table_location(self, table_id: str) -> Extra_Types.HTML_TAG_NAVIGATION_PATH:
        return [{"tag_name": "table", "attributes": {"id": table_id}}]

    def get_default_wrapper_div_location(self, wrapper_div_id: str) -> Extra_Types.HTML_TAG_NAVIGATION_PATH:
        return [{"tag_name": "div", "attributes": {"id": wrapper_div_id, "class": "table_wrapper"}}]


if __name__ == "__main__":
    pass


DEFAULT_FORBIDDEN_CHARS = {",": " ", "\"": ""}

BASE_URL = utils.BASE_URL
DEFAULT_EVADE_SLEEP_MIN: int = 4 # seconds
DEFAULT_EVADE_SLEEP_MAX: int = 7

DEFAULT_PLAYER_TABLE_ROW_FILTERS: list[Extra_Types.TABLE_ROW_FILTER] = [
            {
                "value_location": {
                    "attribute_name": "class"
                },
                "filtered_values": ["thead"]
            }
        ]

DEFAULT_PLAYER_TABLE_EXTRA_ROW_VALS: list[Extra_Types.EXTRA_ROW_VALUE] = [
            {
                "name": "NAME",
                "location": {
                    "attribute_name": "csk",
                    "tag_navigation_path": [
                        {
                            "tag_name": "td",
                            "attributes": {"data-stat": "player"}
                        }
                    ]
                }
            },  
            {
                "name": "ID",
                "location": {
                    "attribute_name": "data-append-csv",
                    "tag_navigation_path": [
                        {
                            "tag_name": "td",
                            "attributes": {"data-stat": "player"}
                        }
                    ]
                }
            },
            {
                "name": "URL",
                "location": {
                    "attribute_name": "href",
                    "tag_navigation_path": [
                        {
                            "tag_name": "td",
                            "attributes": {"data-stat": "player"}
                        },
                        {
                            "tag_name": "a"
                        }
                    ]
                }
            }
        ]