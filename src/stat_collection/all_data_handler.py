# This file serves as an interface between local_database_interface and baseball_reference_client,
# using tools from both to scrape and save data.

from baseball_reference_client import Scraping_Client
import local_database_interface as DI
import utils

def scrape_and_save_team_data(team_url: str, team_abbreviation: str, year: int, overwrite_data: bool = True) -> bool:
    client = Scraping_Client()
    year_page_url = client._scrape_team_page_for_roster_url(team_url, year)

    if year_page_url != None:
        print(f"Saving {year} {team_abbreviation} data...")
        if (DI.find_missing_team_data_files(team_abbreviation, year)) or overwrite_data:
            DI.create_team_year_folder(team_abbreviation, year)
            team_data = client.scrape_team_data(year_page_url)
            
            DI.save_team_data_file(team_abbreviation, year, team_data["roster"], "roster")
            DI.save_team_data_file(team_abbreviation, year, team_data["batting"], "batting")
            DI.save_team_data_file(team_abbreviation, year, team_data["pitching"], "pitching")
            DI.save_team_data_file(team_abbreviation, year, team_data["team_info"], "team_info")
            DI.save_team_data_file(team_abbreviation, year, team_data["common_batting_orders"], "common_batting_orders")
            DI.save_team_data_file(team_abbreviation, year, team_data["schedule"], "schedule")
        
        return True
    
    else:
        return False


def scrape_and_save_player_data(player_page_url: str, player_id: str, stat_types: list[DI.PLAYER_STAT_TYPES], overwrite_data: bool = True) -> bool:
    client = Scraping_Client()
    if utils.BASE_URL not in player_page_url:
        player_page_url = utils.BASE_URL + player_page_url

    for stat_type in stat_types:
        if (not DI.player_data_file_exists(player_id, stat_type)) or overwrite_data:
            player_data = client.scrape_player_stats(player_page_url, stat_type)
            DI.save_player_data_file(player_id, stat_type, player_data)

    return True


def scrape_and_save_league_data(year: int, stat_types: list[DI.LEAGUE_DATA_FILE_TYPES], overwrite_data: bool = True) -> bool:
    client = Scraping_Client()
    print(f"Scraping {year} League average statistics...")
    DI.create_league_year_dir(year)

    for stat_type in stat_types:
        if not DI.league_data_file_exists(stat_type, year) or overwrite_data:
            league_data = client.scrape_league_avg_table(year, stat_type)
            DI.save_league_data_file(year, stat_type, league_data)
    
    return True