from baseball_reference_client import Scraping_Client
import local_database_interface as DI
import utils

def main():
    save_all_rosters_from_year(utils.get_current_year())

def save_all_rosters_from_year(year: str) -> None:
    client = Scraping_Client()
    teams = DI.get_all_teams()
    year = str(year)

    for team_name, team_url, team_abbrev in teams:
        year_page = client.scrape_team_page_for_roster_url(team_url, year)
        if year_page != None:
            print(f"Saving {year} {team_name} roster...")
            DI.create_team_year_folder(team_abbrev, year)
            roster = client.scrape_team_roster_page(year_page)
            DI.save_team_roster_file(team_abbrev, year, roster)

if __name__ == "__main__":
    main()