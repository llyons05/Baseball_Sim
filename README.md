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
- The player stats in all categories (pitching, batting, baserunning, appearances) for that team.

Running `main.py` will give you a series of prompts in the command line which will allow you to do this.

## Running the Simulation
Before running a simulation, you obviously need to have C++ and Make installed. Once these are installed, it's pretty straightforward.
First, you need to navigate to `src/baseball_sim`. Then, build the project using the `make` command.
The only prerequisite to running a simulation is having scraped all the necessary data for the teams you are planning to simulate.
Once that data is scraped, all you need to do is run `.\simulation.exe`, which will prompt you to type in the team that you want to simulate.
It will then ask how many games you want, and after that, it will run the simulation and print out the results!

If you want a more detailed version of the games, build the project with the command `make debug` instead.
