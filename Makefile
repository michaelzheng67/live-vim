# Compiler and flags
CXX = g++
CXXFLAGS = -Wall -std=c++17 -O3

ifeq ($(DEBUG), 1)
  CXXFLAGS += -DDEBUG
endif

# Boost flags
BOOST_INCLUDE = -I/opt/homebrew/include
BOOST_LDFLAGS = -L/opt/homebrew/lib
BOOST_LIBS = -lboost_system -lboost_thread 

ifeq ($(DOCKER), 1)
BOOST_LIBS += -pthread -lrt
endif

# GTest flags
GTEST_INCLUDE = -I/opt/homebrew/include
GTEST_LDFLAGS = -L/opt/homebrew/lib
GTEST_LIBS = -lgtest -lgtest_main

# Linker flags
LDFLAGS = -lncurses $(BOOST_LIBS) $(BOOST_LDFLAGS)

# Directories
SRC_DIR = src
OBJ_DIR = build/obj
BIN_DIR = build
TARGET = $(BIN_DIR)/live-vim

TEST_DIR = test
TEST_OBJ_DIR = build/test_obj
TEST_BIN = $(BIN_DIR)/test-runner

# Source and object files
rwildcard = $(foreach d,$(wildcard $1*),$(call rwildcard,$d/,$2) $(filter $(subst *,%,$2),$d))
SRCS = $(call rwildcard, $(SRC_DIR)/, *.cpp)
OBJS = $(patsubst $(SRC_DIR)/%.cpp, $(OBJ_DIR)/%.o, $(SRCS))
OBJS_WITHOUT_MAIN = $(filter-out $(OBJ_DIR)/main.o, $(OBJS))

TEST_SRCS = $(shell find $(TEST_DIR) -name '*.cpp')
TEST_OBJS = $(patsubst $(TEST_DIR)/%, $(TEST_OBJ_DIR)/%, $(TEST_SRCS:.cpp=.o))

# Default target
all: $(TARGET)

# Link object files to build final binary
$(TARGET): $(OBJS)
	@mkdir -p $(BIN_DIR)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)

# Compile source files
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) $(BOOST_INCLUDE) -c $< -o $@

# Compile test files (recursively)
$(TEST_OBJ_DIR)/%.o: $(TEST_DIR)/%.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) $(BOOST_INCLUDE) $(GTEST_INCLUDE) -c $< -o $@

# Test build and run
test:
	$(MAKE) --always-make DEBUG=1 $(TEST_BIN)
	@./$(TEST_BIN) --gtest_color=yes

$(TEST_BIN): $(TEST_OBJS) $(OBJS_WITHOUT_MAIN)
	@mkdir -p $(BIN_DIR)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS) $(GTEST_LDFLAGS) $(GTEST_LIBS)

# Clean
clean:
	rm -rf $(OBJ_DIR) $(TEST_OBJ_DIR) $(BIN_DIR)/live-vim $(TEST_BIN)

.PHONY: all clean test
