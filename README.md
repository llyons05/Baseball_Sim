# INSTRUCTIONS
There are two parts to this project: **baseball_sim** and **stat_collection**.
## Stat Collection  
Before you can do any actual simulation, you have to get the stat collection part of this project working. 
  
- First, navigate to `src/stat_collection`.
- Then, create a Python virtual environment.
- After this, run the following command: `pip install -r requirements.txt`. This will install all the necessary packages.
 
Now, you're ready to start scraping stats. To run a simulation with a team, you will need to scrape:
- The league stats for that team's year
- The team stats for that team from that year
- The player stats in all categories for all players on that team.
  
You can verify that you have all the needed data for a team/season with the `audit` section of the tool.  

Running `main.py` will give you a series of prompts in the command line which will allow you to do this.
The easiest way to scrape data with the CLI is to navigate to the desired scraping tool, select the team/year, and then just press enter when presented with the selection of stat types.
Scraping an entire season's worth of stats is as easy as:
```
> scrape data, view data, or audit? (s/v/a): s
> scrape team statistics or player statistics or league statistics? (t/p/l): p
> Press enter to save all teams, or type comma separated teams (ex: NYY, WSN, PIT): (press enter)
> Input year to get data from: 2024
> What player stats should be scraped? Press enter for all or select from (batting/...): (press enter)
> Should previously saved data be used when available? (y/n): y
```  
Be aware, scraping stats for a whole season means scraping stats for several hundred players, so if you are doing it for the first time it can take several hours (~10 mins/team * 30 teams).
This is somewhat remedied by the fact that when we save player stats, we are saving stats for their whole career, so once you have them you will never have to scrape them again (unless you want them updated).
For example, if you tried to scrape stats for every player in the 2024 season immediately after scraping the 2023 season, it would take nowhere near as long, since most of the players, having also played in 2023, would already be saved.
However, if an MLB season is currently in progress and you are trying to scrape stats (league/team/player) for it, you should always get the most up-to-date data. In other words, you should always respond **no** to the prompt
`> Should previously saved data be used when available? (y/n):`. Otherwise, the scraper will just see that stats are saved locally for that player and it won't update them with the newest data.

## Running the Simulation
Before running a simulation, you obviously need to have C++ and Make installed. Once these are installed, it's pretty straightforward.
First, you need to navigate to `src/baseball_sim`. Then, build the project using the `make` command.
Now, run `.\simulation.exe`, which will prompt you to choose what kind of simulation you want to run.
However, for whatever reason I can't get this to work on Linux, so for now **it only works on Windows**.
There are two types of simulations you can run: **individual games**, and **full seasons**. 
- **Individual Games**
  - You only need stats pertaining to the two teams you want to simulate, as well as the league stats for that year. You can confirm that you have them by auditing the teams in the Python tool.
  - You will be prompted to type in which two teams to simulate, and how many games they should play against each other. The simulation can currently do ~2000 games/second depending on your machine.
  - If you want a more detailed version of the games, build the project with the command `make view` instead (although you can only view one or two games with this).
- **Full Seasons**
  - You need stats for every team from that season, which can also be checked in the audit tool.
  - You will be prompted to type in the season that you want to simulate, and how many times it should simulate that season. If you choose to simulate the season more than one time, results of all the simulated seasons will be averaged when they are printed out.
