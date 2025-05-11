TARGET = simulation.exe
BUILD_DIR = build
CXX = g++

CXXFLAGS = -g -march=native -Wall -O1
DEBUG_FLAGS = -DBASEBALL_DEBUG=1
VIEWING_FLAGS = -DBASEBALL_VIEW=1

_OBJ_FILES = main.o user_interface.o utils.o statistics.o probability.o team.o season.o load_stats.o game_states.o baseball_game.o
OBJ_FILES = $(patsubst %.o,$(BUILD_DIR)/%.o,$(_OBJ_FILES))
HEADER_FILES = baseball_game.hpp config.hpp game_states.hpp load_stats.hpp player.hpp probability.hpp season.hpp statistics.hpp table.hpp team.hpp user_interface.hpp utils.hpp

ifdef OS # Check if we are on windows
	RD = rd /s /q
	DIR_GUARD = if not exist $(BUILD_DIR) mkdir $(BUILD_DIR)
else
    RD = rm -f
    DIR_GUARD = mkdir -p $(BUILD_DIR)
endif

sim: $(OBJ_FILES)
	$(CXX) $(CXXFLAGS) $^ -o $(TARGET)

debug: $(patsubst %.o,%_debug.o,$(OBJ_FILES))
	$(CXX) $(CXXFLAGS) $^ -o $(TARGET)

view: $(patsubst %.o,%_view.o,$(OBJ_FILES))
	$(CXX) $(CXXFLAGS) $^ -o $(TARGET)

debug_view: $(patsubst %.o,%_debug_view.o,$(OBJ_FILES))
	$(CXX) $(CXXFLAGS) $^ -o $(TARGET)


$(BUILD_DIR)/%.o: %.cpp $(HEADER_FILES)
	@$(DIR_GUARD)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(BUILD_DIR)/%_debug.o: %.cpp $(HEADER_FILES)
	@$(DIR_GUARD)
	$(CXX) $(CXXFLAGS) $(DEBUG_FLAGS) -c $< -o $@

$(BUILD_DIR)/%_view.o: %.cpp $(HEADER_FILES)
	@$(DIR_GUARD)
	$(CXX) $(CXXFLAGS) $(VIEWING_FLAGS) -c $< -o $@

$(BUILD_DIR)/%_debug_view.o: %.cpp $(HEADER_FILES)
	@$(DIR_GUARD)
	$(CXX) $(CXXFLAGS) $(DEBUG_FLAGS) $(VIEWING_FLAGS) -c $< -o $@

clean:
	$(RD) $(BUILD_DIR)