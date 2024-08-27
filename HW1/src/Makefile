CC := gcc
CFLAGS := -Wall
INCLUDES := -I../include
TP := test_programs

all: jobCommander jobExecutorServer jobs  # Waiting_Queue Running_Queue

jobs: hello_world capital_lowercase progDelay

jobExecutorServer: jobExecutorServer.o JES_helping_functions.o Waiting_Queue.o Running_Queue.o
	clear
	$(CC) $(CFLAGS) $(INCLUDES) -o $@ $^

jobCommander: jobCommander.o
	clear
	$(CC) $(CFLAGS) $(INCLUDES) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

Waiting_Queue: Waiting_Queue.o
	clear
	$(CC) $(CFLAGS) $(INCLUDES) -o $@ $^

Running_Queue: Running_Queue.o
	clear
	$(CC) $(CFLAGS) $(INCLUDES) -o $@ $^

progDelay: $(TP)/progDelay.o
	clear
	$(CC) $(CFLAGS) $(INCLUDES) -o $@ $^

hello_world: $(TP)/hello_world.o
	clear
	$(CC) $(CFLAGS) $(INCLUDES) -o $@ $^

capital_lowercase: $(TP)/capital_lowercase.o
	clear
	$(CC) $(CFLAGS) $(INCLUDES) -o $@ $^


valgrind: jobExecutorServer
	clear
	valgrind --leak-check=full --show-leak-kinds=all ./jobExecutorServer
# valgrind --leak-check=full --show-leak-kinds=all ./Waiting_Queue
# valgrind --leak-check=full --show-leak-kinds=all ./Running_Queue

clean:
	$(RM) jobExecutorServer Waiting_Queue Running_Queue jobCommander jobExecutorServer.txt jobCommander_to_jobExecutorServer jobExecutorServer_to_jobCommander hello_world progDelay capital_lowercase*.o

new: clean all
