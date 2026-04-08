# Variables
CXX = g++
CXXFLAGS = -O3 -g
TARGET = bench
SRC = Arena.cpp

# --- FLAMEGRAPH TOOLS ---
# We expect these to be in the PATH or in a specific directory
# In GitHub Actions, we will clone them
FLAMEGRAPH_DIR = ./FlameGraph
COLLAPSE = $(FLAMEGRAPH_DIR)/stackcollapse-perf.pl
FLAMEGRAPH = $(FLAMEGRAPH_DIR)/flamegraph.pl

.PHONY: all clean record-arena record-new record-no_reserve flame-arena flame-new flame-no_reserve benchmark

all: $(TARGET)

$(TARGET): $(SRC)
	$(CXX) $(CXXFLAGS) $(SRC) -o $(TARGET)

benchmark: $(TARGET)
	./$(TARGET) all

# --- RECORDING DATA ---
record-arena: $(TARGET)
	perf record -g -o perf_arena.data ./$(TARGET) arena

record-new: $(TARGET)
	perf record -g -o perf_new.data ./$(TARGET) new

record-no_reserve: $(TARGET)
	perf record -g -o perf_no_reserve.data ./$(TARGET) no_reserve

# --- GENERATING FLAME GRAPHS ---
flame-arena: record-arena
	perf script -i perf_arena.data | $(COLLAPSE) | $(FLAMEGRAPH) > arena_flamegraph.svg
	@echo "Generated arena_flamegraph.svg"

flame-new: record-new
	perf script -i perf_new.data | $(COLLAPSE) | $(FLAMEGRAPH) > new_flamegraph.svg
	@echo "Generated new_flamegraph.svg"

flame-no_reserve: record-no_reserve
	perf script -i perf_no_reserve.data | $(COLLAPSE) | $(FLAMEGRAPH) > no_reserve_flamegraph.svg
	@echo "Generated no_reserve_flamegraph.svg"

flamegraphs: flame-arena flame-new flame-no_reserve

clean:
	rm -f $(TARGET) perf*.data *.svg
	@echo "Cleaned up binary, perf data, and SVGs."
