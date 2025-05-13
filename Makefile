# Makefile for CSE109 Final Exam Server

# Compiler settings
CXX      := g++
CXXFLAGS := -std=c++11 -Iinclude -Isrc -Wall -Wextra -g

# Directories
SRCDIR   := src
OBJDIR   := build/obj
BINDIR   := build/bin

# Server target
TARGET   := $(BINDIR)/fileserver

# Source files (exclude test_client.cpp)
SRCS     := $(wildcard $(SRCDIR)/*.cpp) $(SRCDIR)/hashmap.cpp
SRCS     := $(filter-out $(SRCDIR)/test_client.cpp,$(SRCS))
OBJS     := $(patsubst $(SRCDIR)/%.cpp,$(OBJDIR)/%.o,$(SRCS))

.PHONY: all test test_client test_request install clean

# Default build
all: $(TARGET)

# Link server
$(TARGET): $(OBJS)
	mkdir -p $(BINDIR)
	$(CXX) $(CXXFLAGS) $^ -o $@

# Compile sources
$(OBJDIR)/%.o: $(SRCDIR)/%.cpp
	mkdir -p $(OBJDIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# -------------------------------------------------------------------
# Protocol tests
# -------------------------------------------------------------------
test: $(BINDIR)/test_protocol
	@echo "Running protocol tests..."
	@$(BINDIR)/test_protocol

$(BINDIR)/test_protocol: tests/test_protocol.cpp src/protocol.cpp src/pack109.cpp
	mkdir -p $(BINDIR)
	$(CXX) $(CXXFLAGS) $^ -o $@

# -------------------------------------------------------------------
# Test client
# -------------------------------------------------------------------
test_client: $(BINDIR)/test_client
	@echo "Built test_client: $<"

$(BINDIR)/test_client: src/test_client.cpp src/protocol.cpp src/pack109.cpp
	mkdir -p $(BINDIR)
	$(CXX) $(CXXFLAGS) $^ -o $@
	
# -------------------------------------------------------------------
# Test missing-file error
# -------------------------------------------------------------------
test_error: all
	@mkdir -p $(BINDIR)
	$(CXX) $(CXXFLAGS) tests/test_error.cpp src/protocol.cpp src/pack109.cpp -o $(BINDIR)/test_error
	@echo "Built test_error: $(BINDIR)/test_error"

# -------------------------------------------------------------------
# Request-message tests
# -------------------------------------------------------------------
test_request: $(BINDIR)/test_request
	@echo "Built test_request: $<"

$(BINDIR)/test_request: tests/test_request.cpp src/protocol.cpp src/pack109.cpp
	mkdir -p $(BINDIR)
	$(CXX) $(CXXFLAGS) $^ -o $@

# -------------------------------------------------------------------
# Install
# -------------------------------------------------------------------
install: all
	@cp $(TARGET) bin/fileserver
	@echo "Installed new fileserver → bin/fileserver"

# -------------------------------------------------------------------
# Clean build artifacts
# -------------------------------------------------------------------
clean:
	rm -rf build