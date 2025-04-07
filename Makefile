# Compiler and flags
CXX = g++
CXXFLAGS = -Wall -std=c++17 -O3

# uncomment for debug mode
# CXXFLAGS += -DDEBUG

# Boost flags
BOOST_INCLUDE = -I/opt/homebrew/include
BOOST_LDFLAGS = -L/opt/homebrew/lib
BOOST_LIBS = -lboost_system -lboost_thread

# Linker flags
LDFLAGS = -lncurses $(BOOST_LIBS) $(BOOST_LDFLAGS)

# Directories
SRC_DIR = src
OBJ_DIR = build/obj
BIN_DIR = build
TARGET = $(BIN_DIR)/live-vim

# Recursive wildcard function to get all .cpp files in src and subdirectories
rwildcard = $(foreach d,$(wildcard $1*),$(call rwildcard,$d/,$2) $(filter $(subst *,%,$2),$d))
SRCS = $(call rwildcard, $(SRC_DIR)/, *.cpp)

# Convert source paths to object paths (src/foo/bar.cpp -> build/obj/foo/bar.o)
OBJS = $(patsubst $(SRC_DIR)/%.cpp, $(OBJ_DIR)/%.o, $(SRCS))

# Default target
all: $(TARGET)

# Link object files to build final binary
$(TARGET): $(OBJS)
	@mkdir -p $(BIN_DIR)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)

# Compile each .cpp into corresponding .o file under build/obj
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) $(BOOST_INCLUDE) -c $< -o $@

# Clean target
clean:
	rm -rf $(OBJ_DIR) $(BIN_DIR)/live-vim

.PHONY: all clean
