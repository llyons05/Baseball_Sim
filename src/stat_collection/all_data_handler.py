# This file serves as an interface between local_database_interface and baseball_reference_client,
# using tools from both to scrape and save data.

from baseball_reference_client import Scraping_Client
import local_database_interface as DI
import utils

def scrape_and_save_team_data(team_url: str, team_abbreviation: str, year: int, overwrite_data: bool = True) -> bool:
    client = Scraping_Client()
    year_page_url = client.scrape_team_page_for_roster_url(team_url, year)

    if year_page_url != None:
        print(f"Saving {year} {team_abbreviation} data...")

        if (not (DI.team_data_file_exists(team_abbreviation, year, "roster") and DI.team_data_file_exists(team_abbreviation, year, "pitching"))) or overwrite_data:
            DI.create_team_year_folder(team_abbreviation, year)
            team_data = client.scrape_team_data(year_page_url)
            
            DI.save_team_roster_file(team_abbreviation, year, team_data["roster"])
            DI.save_team_pitching_file(team_abbreviation, year, team_data["pitching"])
        
        return True
    
    else:
        return False


def scrape_and_save_player_data(player_page_url: str, player_id: str, stat_type: DI.STAT_TYPES, overwrite_data: bool = True) -> bool:
    client = Scraping_Client()

    if (not DI.player_data_file_exists(player_id, stat_type)) or overwrite_data:
        player_data = client.scrape_player_stats(player_page_url, stat_type)
        DI.save_player_data_file(player_id, stat_type, player_data)

    return True


