from bs4 import BeautifulSoup
import urllib.request
import time
import random
from typing import Literal

import utils
import local_database_interface as DI
from table_parser import Table_Parser, NoTableFoundException
import table_parsing_types as Extra_Types
from table import Table, EMPTY_TABLE

class Scraping_Client:

    def __init__(self):
        self.html_cache = dict()


    def evade(self) -> None:
        time.sleep(random.randint(DEFAULT_EVADE_SLEEP_MIN, DEFAULT_EVADE_SLEEP_MAX))


    def _scrape_page_html(self, url: str):
        if self._is_in_cache(url):
            return self.html_cache[url]

        self.evade()
        try:
            with urllib.request.urlopen(url) as response:
                html = response.read()
                self.html_cache[url] = html
                return html
        except:
            return None


    def scrape_team_data(self, team_roster_page_url: str) -> dict[DI.TEAM_DATA_FILE_TYPES, Table]:
        response = self._scrape_page_html(team_roster_page_url)

        result: dict[DI.TEAM_DATA_FILE_TYPES, Table] = dict()
        result["roster"] = self._parse_team_roster_table(response)
        result["batting"] = self._parse_team_batting_table(response)
        result["pitching"] = self._parse_team_pitching_table(response)
        result["team_info"] = self._parse_team_info_table(response, team_roster_page_url)
        result["common_batting_orders"] = self._get_team_common_batting_orders_table(team_roster_page_url)
        result["schedule"] = self._get_team_schedule_table(team_roster_page_url)

        return result


    def _parse_team_roster_table(self, main_team_roster_page_html) -> Table:
        """
        Takes in the html response from the main team page that has that year's roster on it.
        Returns a Table containing the roster data for the team.
        """
        roster = self._parse_default_player_list_table(main_team_roster_page_html, "appearances", "all_appearances")
        for row in roster.rows:
            row["ID"] = utils.get_player_id_from_url(row["URL"])

        return roster


    def _parse_team_pitching_table(self, main_team_roster_page_html) -> Table:
        """
        Takes in the html response from the main team page that has that year's roster on it.
        Returns a Table containing the team's list of pitchers and some basic stats for them.
        """

        pitchers = self._parse_default_player_list_table(main_team_roster_page_html, "players_standard_pitching", "all_players_standard_pitching")
        for row in pitchers.rows:
            if not row["team_position"]:
                row["team_position"] = "P"

        return pitchers


    def _parse_team_batting_table(self, main_team_roster_page_html) -> Table:
        """
        Takes in the html response from the main team page that has that year's roster on it.
        Returns a Table containing the team's batting stats.
        """

        batters = self._parse_default_player_list_table(main_team_roster_page_html, "players_standard_batting", "all_players_standard_batting")
        return batters


    def _parse_default_player_list_table(self, team_page_html, table_id: str, table_parent_div_id: str) -> Table:
        """
        Takes in the html response from a team page with the desired table of players on it, as well as
        the "id" of the table in the html, and the "id" of the div that contains the table.
        Returns a parsed version of that table. This method can be used for most tables of players on baseball reference.
        """

        parser = Table_Parser(team_page_html, self._get_default_table_location(table_id), self._get_default_wrapper_div_location(table_parent_div_id))
        table_data = parser.parse(DEFAULT_PLAYER_TABLE_EXTRA_ROW_VALS, DEFAULT_PLAYER_TABLE_ROW_FILTERS, ["player", "ranker"], DEFAULT_FORBIDDEN_CHARS)
        return table_data


    def _parse_team_info_table(self, main_team_roster_page_html, team_year_page_url: str) -> Table:
        """
        Takes in the html response from the main team page that has that year's roster on it, as well as the url to that page.
        Returns a table with basic info about the team.
        """

        result = Table()
        team_abbreviation = utils.get_abbreviation_from_specific_team_page_url(team_year_page_url)

        result.add_row({"abbreviation": team_abbreviation}, True)

        return result


    def _get_team_common_batting_orders_table(self, team_roster_page_url: str) -> Table:
        """
        Takes in the url to main team page that has that year's roster on it.
        Returns a table with the most common batting orders for that team.
        """

        url = utils.get_team_batting_order_url(team_roster_page_url)
        response = self._scrape_page_html(url)

        wrapper_div_location = [{"tag_name": "div", "attributes": {"id": "all_common_orders"}}]
        table_location = [wrapper_div_location[0], {"tag_name": "table", "attributes": {"class": "stats_table"}}]
        parser = Table_Parser(response, table_location, wrapper_div_location, table_row_tag_name="td", table_body_tag_name="tr",
                              row_cell_tag_name="li", cell_descriptor_attribute_name="value", cell_parsing_method=self._parse_batting_order_cell)

        extra_row_values: list[Extra_Types.EXTRA_ROW_VALUE] = [
            {"name": "games", "location": {"tag_navigation_path": [{"tag_name": "strong"}]}}
        ]

        table = parser.parse(extra_row_values, forbidden_chars={" Games": ""})
        return table


    def _get_team_schedule_table(self, main_team_roster_page_url: str) -> Table:
        """
        Takes in the url to main team page that has that year's roster on it.
        Returns a table with the schedule of that team for that year.
        """
        url = utils.get_team_schedule_url(main_team_roster_page_url)
        response = self._scrape_page_html(url)
        parser = Table_Parser(response, self._get_default_table_location("team_schedule"), self._get_default_wrapper_div_location("all_team_schedule"))

        table = parser.parse(row_filters=DEFAULT_PLAYER_TABLE_ROW_FILTERS, column_filters=["boxscore", "team_ID", "starttime", "preview"], forbidden_chars=DEFAULT_FORBIDDEN_CHARS)
        return table


    def _parse_batting_order_cell(self, cell: BeautifulSoup) -> str:
        """ Helper method to parse batting order table cells. """

        hyperlink = cell.find("a")
        if hyperlink:
            return hyperlink.get("data-entry-id")
        return cell.find(string=True)


    def _scrape_team_page_for_roster_url(self, team_page_url: str, year: int) -> str | None:
        """
        Takes in the url for the team page that has the list of all the years of the franchise, as well as the year we are looking for.
        Returns the url to the team page of that franchise for that year, and returns None if no roster exists for that year.
        """

        response = self._scrape_page_html(team_page_url)
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
        table_parser = Table_Parser(response, self._get_default_table_location("franchise_years"), self._get_default_wrapper_div_location("all_franchise_years"))
        franchise_years_table = table_parser.parse(extra_columns)

        search_results = franchise_years_table.search_rows({"year_ID": str(year)})
        if (len(search_results) > 0):
            return BASE_URL + search_results[0]["URL"]

        return None


    def scrape_player_stats(self, base_player_page_url: str, stat_type: DI.PLAYER_STAT_TYPES) -> Table:
        """
        Takes in the url to a player's baseball reference page, as well as the type of stat to be scraped.
        Returns a Table containing the data for that player, and an Empty Table if no table is found.
        """

        table_parser = None
        if stat_type == "batting":
            table_parser = self._try_scraping_batting_tables(base_player_page_url)

        elif stat_type == "pitching":
            table_parser = self._try_scraping_pitching_tables(base_player_page_url)

        elif stat_type == "appearances":
            table_parser = self._scrape_table_from_player_page(base_player_page_url, "appearances", "all_appearances")

        elif stat_type == "baserunning":
            table_parser = self._scrape_table_from_player_page(utils.get_player_batting_page_url(base_player_page_url), "batting_baserunning", "all_batting_baserunning")
    
        elif stat_type == "batting_against":
            table_parser = self._scrape_table_from_player_page(utils.get_player_pitching_page_url(base_player_page_url), "pitching_batting", "all_pitching_batting")

        if table_parser is None:
            return EMPTY_TABLE

        row_filters: list[Extra_Types.TABLE_ROW_FILTER] = [
            {
                "value_location": {"attribute_name": "class"},
                "filtered_values": ["hidden", "spacer", "nonroster_table"]
            }
        ]

        player_data_table = table_parser.parse(row_filters=row_filters, forbidden_chars=DEFAULT_FORBIDDEN_CHARS)
        player_data_table.sort(key=lambda d: int(d.get("year_ID", 0)))
        return player_data_table


    def _try_scraping_batting_tables(self, base_player_page_url: str) -> Table_Parser | None:
        player_batting_page_url = utils.get_player_batting_page_url(base_player_page_url)
        table_id = "players_standard_batting"
        wrapper_div_id = "all_players_standard_batting"

        table_parser = self._scrape_table_from_player_page(player_batting_page_url, table_id, wrapper_div_id)

        if table_parser is None:
            table_parser = self._scrape_table_from_player_page(base_player_page_url, table_id, wrapper_div_id)
        
        return table_parser


    def _try_scraping_pitching_tables(self, base_player_page_url: str) -> Table_Parser | None:
        player_pitching_page_url = utils.get_player_pitching_page_url(base_player_page_url)
        table_id = "players_standard_pitching"
        wrapper_div_id = "all_players_standard_pitching"

        table_parser = self._scrape_table_from_player_page(player_pitching_page_url, table_id, wrapper_div_id)

        if table_parser is None:
            table_parser = self._scrape_table_from_player_page(base_player_page_url, table_id, wrapper_div_id)

        return table_parser


    def _scrape_table_from_player_page(self, player_page_url: str,
                                      table_id: str,
                                      table_wrapper_div_id: str) -> Table_Parser | None:
        response = self._scrape_page_html(player_page_url)
        if response is None:
            return None

        try:
            table_parser = Table_Parser(response, self._get_default_table_location(table_id), self._get_default_wrapper_div_location(table_wrapper_div_id))

        except NoTableFoundException as e:
            print(f"NO TABLE FOUND ON PAGE {player_page_url}")
            return None
        
        return table_parser


    def scrape_league_avg_table(self, year: int, stat_type: DI.LEAGUE_DATA_FILE_TYPES) -> Table:
        base_league_year_url = utils.get_base_league_year_url(year)
        table = None
        if stat_type == "batting":
            table = self._scrape_default_league_avg_table(base_league_year_url, "teams_standard_batting", "all_teams_standard_batting")
        elif stat_type == "pitching":
            table = self._scrape_default_league_avg_table(base_league_year_url, "teams_standard_pitching", "all_teams_standard_pitching")
        elif stat_type == "standings":
            table = self._scrape_league_standings_table(base_league_year_url)

        if table == None:
            return EMPTY_TABLE

        return table


    def _scrape_league_standings_table(self, base_league_year_url: str) -> Table | None:
        url = utils.get_league_standings_url(base_league_year_url)
        table_location = self._get_default_table_location("expanded_standings_overall")
        wrapper_div_location = self._get_default_wrapper_div_location("all_expanded_standings_overall")

        response = self._scrape_page_html(url)
        if response is None:
            return None

        table_parser = Table_Parser(response, table_location, wrapper_div_location)

        row_filters = [{"value_location": {"attribute_name": "class"},"filtered_values": ["league_average_table"]}]
        row_filters.extend(DEFAULT_PLAYER_TABLE_ROW_FILTERS)

        extra_vals: list[Extra_Types.EXTRA_ROW_VALUE] = [{ "name": "URL",
                "location": {
                    "attribute_name": "href",
                    "tag_navigation_path": [
                        {"tag_name": "td", "attributes": {"data-stat": "team_name"}},
                        {"tag_name": "a"}]
                }
            }]

        table = table_parser.parse(extra_vals, row_filters, ["ranker"], DEFAULT_FORBIDDEN_CHARS)
        table.add_header("ID")
        for row in table.rows:
            row["ID"] = utils.get_team_id_from_url(row["URL"])
        
        return table


    def _scrape_default_league_avg_table(self, league_stat_url: str, table_id: str, wrapper_div_id: str) -> Table | None:
        table_location = self._get_default_table_location(table_id)
        wrapper_div_location = self._get_default_wrapper_div_location(wrapper_div_id)

        response = self._scrape_page_html(league_stat_url)
        if response is None:
            return None
        
        table_parser = Table_Parser(response, table_location, wrapper_div_location, table_body_tag_name="tfoot")
        table = table_parser.parse(column_filters=["team_name"], forbidden_chars=DEFAULT_FORBIDDEN_CHARS)
        return table


    def _is_in_cache(self, url: str) -> bool:
        return url in self.html_cache.keys()

    def _get_default_table_location(self, table_id: str) -> Extra_Types.HTML_TAG_NAVIGATION_PATH:
        return [{"tag_name": "table", "attributes": {"id": table_id}}]

    def _get_default_wrapper_div_location(self, wrapper_div_id: str) -> Extra_Types.HTML_TAG_NAVIGATION_PATH:
        return [{"tag_name": "div", "attributes": {"id": wrapper_div_id, "class": "table_wrapper"}}]



DEFAULT_FORBIDDEN_CHARS = {",": " ", "\"": "", "%": ""}

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
                "name": "ID",
                "location": {
                    "attribute_name": "data-append-csv",
                    "tag_navigation_path": [
                        {
                            "tag_name": "td",
                            "attributes": {"data-stat": "name_display"}
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
                            "attributes": {"data-stat": "name_display"}
                        },
                        {
                            "tag_name": "a"
                        }
                    ]
                }
            }
        ]