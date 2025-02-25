CFLAGS = -g -march=native -Wall
DEBUG_FLAGS = -DBASEBALL_DEBUG=1
SOURCE_FILES = main.cpp table.cpp statistics.cpp probability.cpp team.cpp load_stats.cpp game_states.cpp baseball_game.cpp

sim: 
	g++ $(CFLAGS) -o simulation.exe $(SOURCE_FILES)

debug:
	g++ $(CFLAGS) $(DEBUG_FLAGS) -o simulation.exe $(SOURCE_FILES)