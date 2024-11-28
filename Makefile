# Compiler
CXX = g++

# Compiler flags
CXXFLAGS = -Wall -Wextra -std=c++17

# Default server hostname
SERVER_HOST ?= localhost

# Default server port
SERVER_PORT ?= 8080

# Default client directory
CLIENT_DIR ?= ./tests

# Default i value for tests
TEST_I ?= 8

# Targets
all: client server

# Client compiling
client: client.cpp
	$(CXX) $(CXXFLAGS) -o client client.cpp

# Server compiling
server: server.cpp
	$(CXX) $(CXXFLAGS) -o server server.cpp

# Tests compiling
tests: gen_tests.cpp
	$(CXX) $(CXXFLAGS) -o gen_tests gen_tests.cpp

# Client execution
run-client: client
	./client $(SERVER_HOST) $(SERVER_PORT) $(CLIENT_DIR)

# Server execution
run-server: server
	./server $(SERVER_PORT)

# Tests execution
run-tests: tests
	./gen_tests $(TEST_I)

# Clear binaries
clean:
	rm -f client server

# Rebuild binaries
rebuild: clean all

.PHONY: all client server run-client run-server clean rebuild