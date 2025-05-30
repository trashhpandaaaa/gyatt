# Makefile for Gyatt

CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -pedantic -Iinclude -O3 -march=native -mtune=native -mavx2 -pthread
LDFLAGS = -lssl -lcrypto -lcurl -ljsoncpp -pthread -lz -llz4

SRCDIR = src
INCDIR = include
BUILDDIR = build
SOURCES = $(wildcard $(SRCDIR)/*.cpp)
OBJECTS = $(SOURCES:$(SRCDIR)/%.cpp=$(BUILDDIR)/%.o)
TARGET = gyatt

.PHONY: all clean install

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CXX) $(OBJECTS) -o $@ $(LDFLAGS)

$(BUILDDIR)/%.o: $(SRCDIR)/%.cpp | $(BUILDDIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(BUILDDIR):
	mkdir -p $(BUILDDIR)

clean:
	rm -rf $(BUILDDIR) $(TARGET)

install: $(TARGET)
	cp $(TARGET) /usr/local/bin/

# CMake targets
cmake-build:
	mkdir -p build-cmake
	cd build-cmake && cmake .. && make

cmake-clean:
	rm -rf build-cmake

.PHONY: cmake-build cmake-clean
