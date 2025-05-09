# Makefile for CSE109 Final Exam Server
# ======================================

# Compiler settings
CXX      := g++
CXXFLAGS := -std=c++11 -Iinclude -Isrc -Wall -Wextra -g

# Directories
SRCDIR   := src
OBJDIR   := build/obj
BINDIR   := build/bin

# Our server target (will overwrite the provided bin/fileserver when you run `make install`)
TARGET   := $(BINDIR)/fileserver

# Source files: include your protocol, Pack109, and hashmap implementations
# Exclude test_client.cpp so server doesn't link test client main
SRCS     := $(filter-out src/test_client.cpp,$(wildcard $(SRCDIR)/*.cpp)) src/hashmap.cpp
OBJS     := $(patsubst $(SRCDIR)/%.cpp,$(OBJDIR)/%.o,$(SRCS))

# Default build
all: $(TARGET)
all: $(TARGET)

# Build and run protocol tests
test: all
	@mkdir -p $(BINDIR)
	$(CXX) $(CXXFLAGS) tests/test_protocol.cpp src/protocol.cpp src/pack109.cpp -o $(BINDIR)/test_protocol
	@echo "Running protocol tests..."
	@$(BINDIR)/test_protocol

# Build test client
test_client: all
	@mkdir -p $(BINDIR)
	$(CXX) $(CXXFLAGS) src/test_client.cpp src/protocol.cpp src/pack109.cpp -o $(BINDIR)/test_client
	@echo "Built test_client: $(BINDIR)/test_client"

# Link step
$(TARGET): $(OBJS)
	@mkdir -p $(BINDIR)
	$(CXX) $(CXXFLAGS) $^ -o $@

# Compile step
$(OBJDIR)/%.o: $(SRCDIR)/%.cpp
	@mkdir -p $(OBJDIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Install (overwrite provided)
install: all
	@cp $(TARGET) bin/fileserver
	@echo "Installed new fileserver → bin/fileserver"

# Clean build artifacts
clean:
	rm -rf build

.PHONY: all test test_client install clean
