CFLAGS = -g -march=native -Wall -O1
DEBUG_FLAGS = -DBASEBALL_DEBUG=1
VIEWING_FLAGS = -DBASEBALL_VIEW=1
SOURCE_FILES = main.cpp user_interface.cpp utils.cpp statistics.cpp probability.cpp team.cpp season.cpp load_stats.cpp game_states.cpp baseball_game.cpp

sim: 
	g++ $(CFLAGS) -o simulation.exe $(SOURCE_FILES)

view:
	g++ $(CFLAGS) $(VIEWING_FLAGS) -o simulation.exe $(SOURCE_FILES)

debug:
	g++ $(CFLAGS) $(DEBUG_FLAGS) -o simulation.exe $(SOURCE_FILES)

debug_and_view:
	g++ $(CFLAGS) $(DEBUG_FLAGS) $(VIEWING_FLAGS) -o simulation.exe $(SOURCE_FILES)