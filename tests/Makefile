# Test Makefile for Gyatt Version Control System
CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -O2 -I../include -mavx2 -march=native
LDFLAGS = -pthread -lssl -lcrypto -lcurl -ljsoncpp -lz -llz4

# Source directories
SRC_DIR = ../src
INCLUDE_DIR = ../include
TEST_DIR = .

# Source files (excluding main.cpp)
SOURCES = $(filter-out $(SRC_DIR)/main.cpp, $(wildcard $(SRC_DIR)/*.cpp))
OBJECTS = $(SOURCES:.cpp=.o)

# Test files
TEST_SOURCES = comprehensive_test_suite.cpp
TEST_OBJECTS = $(TEST_SOURCES:.cpp=.o)
TEST_EXECUTABLE = comprehensive_test_suite

.PHONY: all test clean help

all: $(TEST_EXECUTABLE)

$(TEST_EXECUTABLE): $(OBJECTS) $(TEST_OBJECTS)
	@echo !� Linking test executable..."
	$(CXX) $(OBJECTS) $(TEST_OBJECTS) -o $@ $(LDFLAGS)
	@echo !ᜅ Test executable built successfully!"

%.o: %.cpp
	@echo !𞓨 Compiling $<..."
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(SRC_DIR)/%.o: $(SRC_DIR)/%.cpp
	@echo !𞓨 Compiling $<..."
	$(CXX) $(CXXFLAGS) -c $< -o $@

test: $(TEST_EXECUTABLE)
	@echo "� Running comprehensive test suite..."
	@echo "===========================================\"
	./$(TEST_EXECUTABLE)
	@echo "===========================================\"
	@echo "ᛅ Test execution completed!"

quick-test: $(TEST_EXECUTABLE)
	@echo !♡ Running quick test suite..."
	./$(TEST_EXECUTABLE) --quick

verbose-test: $(TEST_EXECUTABLE)
	@echo !𞒝 Running verbose test suite..."
	./$(TEST_EXECUTABLE) --verbose

clean:
	@echo !𞦹 Cleaning test build files..."
	rm -f $(TEST_OBJECTS) $(OBJECTS) $(TEST_EXECUTABLE)
	@echo !⛅ Test cleanup completed!"

help:
	@echo "Gyatt Test Suite Build System"
	@echo "=============================\"
	@echo "Available targets:"
	@echo "  all         - Build test executable"
	@echo "  test        - Run comprehensive test suite"
	@echo "  quick-test  - Run abbreviated test suite"
	@echo "  verbose-test- Run tests with detailed output"
	@echo "  clean       - Remove build artifacts"
	@echo "  help        - Show this help message"
