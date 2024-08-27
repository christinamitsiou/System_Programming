# Directories
SRC_DIR := src
BUILD_DIR := build
BIN_DIR := bin
TESTS_DIR := tests
INCLUDE_DIR := include

# Compiler and flags
CC := gcc
CFLAGS := -Wall -pthread
LDFLAGS := -pthread
INC := -I$(INCLUDE_DIR)

all: $(BIN_DIR)/jobCommander $(BIN_DIR)/jobExecutorServer $(BIN_DIR)/progDelay $(BIN_DIR)/hello_world $(BIN_DIR)/capital_lowercase

# jobCommander
$(BIN_DIR)/jobCommander: $(BUILD_DIR)/jobCommander.o $(BUILD_DIR)/Queue.o
	$(CC) $(LDFLAGS) -o $@ $^

$(BUILD_DIR)/jobCommander.o: $(SRC_DIR)/jobCommander.c $(INCLUDE_DIR)/Queue.h
	$(CC) $(CFLAGS) $(INC) -c -o $@ $<

# jobExecutorServer
$(BIN_DIR)/jobExecutorServer: $(BUILD_DIR)/jobExecutorServer.o $(BUILD_DIR)/Thread_Functions.o $(BUILD_DIR)/JES_helping_functions.o $(BUILD_DIR)/Queue.o
	$(CC) $(LDFLAGS) -o $@ $^

$(BUILD_DIR)/jobExecutorServer.o: $(SRC_DIR)/jobExecutorServer.c $(INCLUDE_DIR)/Thread_Functions.h $(INCLUDE_DIR)/JES_helping_functions.h $(INCLUDE_DIR)/Queue.h
	$(CC) $(CFLAGS) $(INC) -c -o $@ $<

# Thread_Functions
$(BUILD_DIR)/Thread_Functions.o: $(SRC_DIR)/Thread_Functions.c $(INCLUDE_DIR)/Thread_Functions.h $(INCLUDE_DIR)/Queue.h
	$(CC) $(CFLAGS) $(INC) -c -o $@ $<

# JES_helping_functions
$(BUILD_DIR)/JES_helping_functions.o: $(SRC_DIR)/JES_helping_functions.c $(INCLUDE_DIR)/JES_helping_functions.h $(INCLUDE_DIR)/Queue.h
	$(CC) $(CFLAGS) $(INC) -c -o $@ $<

# Queue
$(BUILD_DIR)/Queue.o: $(SRC_DIR)/Queue.c $(INCLUDE_DIR)/Queue.h
	$(CC) $(CFLAGS) $(INC) -c -o $@ $<

# progDelay
$(BIN_DIR)/progDelay: $(BUILD_DIR)/progDelay.o
	$(CC) $(LDFLAGS) -o $@ $^

$(BUILD_DIR)/progDelay.o: $(TESTS_DIR)/progDelay.c
	$(CC) $(CFLAGS) $(INC) -c -o $@ $<

# hello_world
$(BIN_DIR)/hello_world: $(BUILD_DIR)/hello_world.o
	$(CC) $(LDFLAGS) -o $@ $^

$(BUILD_DIR)/hello_world.o: $(TESTS_DIR)/hello_world.c
	$(CC) $(CFLAGS) $(INC) -c -o $@ $<

# capital_lowercase
$(BIN_DIR)/capital_lowercase: $(BUILD_DIR)/capital_lowercase.o
	$(CC) $(LDFLAGS) -o $@ $^

$(BUILD_DIR)/capital_lowercase.o: $(TESTS_DIR)/capital_lowercase.c
	$(CC) $(CFLAGS) $(INC) -c -o $@ $<

valgrind: $(BIN_DIR)/jobExecutorServer
	clear
	valgrind --leak-check=full --show-leak-kinds=all ./$< 9002 8 3

jobs: $(BIN_DIR)/progDelay $(BIN_DIR)/hello_world $(BIN_DIR)/capital_lowercase

clean:
	@$(RM) $(BIN_DIR)/* $(BUILD_DIR)/*
	clear

new: clean all
