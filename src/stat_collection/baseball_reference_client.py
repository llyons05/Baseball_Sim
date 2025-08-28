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


    def scrape_team_stats(self, team_roster_page_url: str, stat_type: DI.TEAM_DATA_FILE_TYPES) -> Table:
        table = None
        if stat_type == "roster":
            table = self._scrape_team_roster_table(team_roster_page_url)
        elif stat_type == "batting":
            table = self._scrape_team_batting_table(team_roster_page_url)
        elif stat_type == "pitching":
            table = self._scrape_team_pitching_table(team_roster_page_url)
        elif stat_type == "common_batting_orders":
            table = self._scrape_team_common_batting_orders_table(team_roster_page_url)
        elif stat_type == "schedule":
            table = self._scrape_team_schedule_table(team_roster_page_url)
        elif stat_type == "team_info":
            table = self._scrape_team_info_table(team_roster_page_url)
        
        if table is None:
            return EMPTY_TABLE
        
        return table


    def _scrape_team_roster_table(self, team_roster_page_url: str) -> Table:
        """
        Takes in the html response from the main team page that has that year's roster on it.
        Returns a Table containing the roster data for the team.
        """
        roster = self._scrape_default_player_list_table(team_roster_page_url, "appearances", "all_appearances")
        for row in roster.rows:
            row["ID"] = utils.get_player_id_from_url(row["URL"])

        return roster


    def _scrape_team_pitching_table(self, team_roster_page_url: str) -> Table:
        """
        Takes in the html response from the main team page that has that year's roster on it.
        Returns a Table containing the team's list of pitchers and some basic stats for them.
        """

        pitchers = self._scrape_default_player_list_table(team_roster_page_url, "players_standard_pitching", "all_players_standard_pitching")
        for row in pitchers.rows:
            if not row["team_position"]:
                row["team_position"] = "P"

        return pitchers


    def _scrape_team_batting_table(self, team_roster_page_url: str) -> Table:
        """
        Takes in the html response from the main team page that has that year's roster on it.
        Returns a Table containing the team's batting stats.
        """

        batters = self._scrape_default_player_list_table(team_roster_page_url, "players_standard_batting", "all_players_standard_batting")
        return batters


    def _scrape_default_player_list_table(self, table_page_url: str, table_id: str, table_parent_div_id: str) -> Table:
        """
        Takes in the html response from a team page with the desired table of players on it, as well as
        the "id" of the table in the html, and the "id" of the div that contains the table.
        Returns a parsed version of that table. This method can be used for most tables of players on baseball reference.
        """
        response = self._scrape_page_html(table_page_url)
        if response is None:
            return None

        parser = Table_Parser(response, self._get_default_table_location(table_id), self._get_default_wrapper_div_location(table_parent_div_id))
        table_data = parser.parse(DEFAULT_PLAYER_TABLE_EXTRA_ROW_VALS, DEFAULT_PLAYER_TABLE_ROW_FILTERS, ["player", "ranker"], DEFAULT_FORBIDDEN_CHARS)
        return table_data


    def _scrape_team_info_table(self, team_roster_page_url: str) -> Table:
        """
        Takes in the html response from the main team page that has that year's roster on it, as well as the url to that page.
        Returns a table with basic info about the team.
        """

        result = Table()
        team_abbreviation = utils.get_abbreviation_from_specific_team_page_url(team_roster_page_url)

        result.add_row({"abbreviation": team_abbreviation}, True)

        return result


    def _scrape_team_common_batting_orders_table(self, team_roster_page_url: str) -> Table:
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

        table = parser.parse(extra_row_values, forbidden_chars={" Games": "", "1 Game": "1"})
        return table


    def _scrape_team_schedule_table(self, team_roster_page_url: str) -> Table:
        """
        Takes in the url to main team page that has that year's roster on it.
        Returns a table with the schedule of that team for that year.
        """
        url = utils.get_team_schedule_url(team_roster_page_url)
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


    def _scrape_team_index_page_for_roster_url(self, team_index_page_url: str, year: int) -> str | None:
        """
        Takes in the url for the franchise index page that has the list of all the years of the franchise, as well as the year we are looking for.
        Returns the url to the team page of that franchise for that year, and returns None if no roster exists for that year.
        """

        response = self._scrape_page_html(team_index_page_url)
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

        table_parser = self._get_player_stat_table_parser(base_player_page_url, stat_type)

        if table_parser is None:
            return EMPTY_TABLE

        row_filters: list[Extra_Types.TABLE_ROW_FILTER] = [
            {
                "value_location": {"attribute_name": "class"},
                "filtered_values": ["hidden", "spacer", "nonroster_table"]
            }
        ]

        player_data_table = table_parser.parse(row_filters=row_filters, forbidden_chars=DEFAULT_FORBIDDEN_CHARS)
        return player_data_table


    def _get_player_stat_table_parser(self, base_player_page_url: str, stat_type: DI.PLAYER_STAT_TYPES) -> Table_Parser | None:
        if stat_type == "batting":
            return self._scrape_table_from_player_page(base_player_page_url, "players_standard_batting", "all_players_standard_batting")

        elif stat_type == "pitching":
            return self._scrape_table_from_player_page(base_player_page_url, "players_standard_pitching", "all_players_standard_pitching")

        elif stat_type == "fielding":
            return self._scrape_table_from_player_page(base_player_page_url, "players_standard_fielding", "all_players_standard_fielding")

        elif stat_type == "appearances":
            return self._scrape_table_from_player_page(base_player_page_url, "appearances", "all_appearances")

        elif stat_type == "baserunning":
            return self._scrape_table_from_player_page(utils.get_player_batting_page_url(base_player_page_url), "batting_baserunning", "all_batting_baserunning")
        
        elif stat_type == "baserunning_against":
            return self._scrape_table_from_player_page(utils.get_player_pitching_page_url(base_player_page_url), "pitching_basesituation", "all_pitching_basesituation")
    
        elif stat_type == "batting_against":
            return self._scrape_table_from_player_page(utils.get_player_pitching_page_url(base_player_page_url), "pitching_batting", "all_pitching_batting")

        elif stat_type == "situational_batting":
            return self._scrape_table_from_player_page(utils.get_player_batting_page_url(base_player_page_url), "batting_situational", "all_batting_situational")
        
        elif stat_type == "ratio_pitching":
            return self._scrape_table_from_player_page(utils.get_player_pitching_page_url(base_player_page_url), "pitching_ratio", "all_pitching_ratio")
        
        return None



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


    def scrape_league_stats(self, year: int, stat_type: DI.LEAGUE_DATA_FILE_TYPES) -> Table:
        base_league_year_url = utils.get_base_league_year_url(year)
        table = None
        if stat_type == "batting":
            table = self._scrape_default_league_avg_table(base_league_year_url, "teams_standard_batting", "all_teams_standard_batting")

        elif stat_type == "pitching":
            table = self._scrape_default_league_avg_table(base_league_year_url, "teams_standard_pitching", "all_teams_standard_pitching")
        
        elif stat_type == "fielding":
            table = self._scrape_default_league_avg_table(base_league_year_url, "teams_standard_fielding", "all_teams_standard_fielding")

        elif stat_type == "baserunning":
            table = self._scrape_default_league_avg_table(utils.get_league_baserunning_url(base_league_year_url), "teams_baserunning_batting", "all_teams_baserunning_batting")

        elif stat_type == "batting_by_bases":
            table = self._scrape_league_batting_split_table(year, "bases", "all_bases")

        elif stat_type == "situational_batting":
            table = self._scrape_default_league_avg_table(utils.get_league_situational_batting_url(base_league_year_url), "teams_situational_batting", "all_teams_situational_batting")

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


    def _scrape_league_batting_split_table(self, year: int, table_id: str, wrapper_div_id: str) -> Table | None:
        url = utils.get_league_batting_split_url(year)
        response = self._scrape_page_html(url)
        if response is None:
            return None
        
        table_location = self._get_default_table_location(table_id)
        wrapper_div_location = self._get_default_wrapper_div_location(wrapper_div_id)
        table_parser = Table_Parser(response, table_location, wrapper_div_location)
        column_filters = ["incomplete_split"]
        table = table_parser.parse(column_filters=column_filters, forbidden_chars=DEFAULT_FORBIDDEN_CHARS)

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