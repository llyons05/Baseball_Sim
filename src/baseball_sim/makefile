CFLAGS = -g -march=native -Wall
DEBUG_MODE = -DBASEBALL_DEBUG=1

sim: 
	g++ $(CFLAGS) -o simulation.exe main.cpp table.cpp statistics.cpp team.cpp load_stats.cpp game_states.cpp baseball_game.cpp

debug:
	g++ $(CFLAGS) $(DEBUG_MODE) -o simulation.exe main.cpp table.cpp statistics.cpp team.cpp load_stats.cpp game_states.cpp baseball_game.cpp