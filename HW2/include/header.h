#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <sys/socket.h>
#include <sys/wait.h>     
#include <sys/types.h>     
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <signal.h> 
#include <unistd.h> 
#include <ctype.h>
#include <signal.h>
#include <sys/wait.h>
#include <stdbool.h>
#include <errno.h>
#include <fcntl.h>
#include "Queue.h"

// Max number of incoming connections
#define CONNECTIONS 10

// A struct used for the shared data between threads and files
typedef struct Shared_Data {
    int sock;                                   // The initial socket
    struct sockaddr_in client;                  // A sockaddr_in structure to store client address information.
    struct sockaddr* clientptr;                 // A pointer to the client address structure,
    volatile sig_atomic_t exiting;              // A variable that defines if the program is exiting
    volatile sig_atomic_t buf_not_empty;        // A variable that defines if the buffer is not empty (has at least one job)
    volatile sig_atomic_t buf_not_full;         // A variable that defines if the buffer is not full (has space for at least one job)
    pthread_mutex_t exiting_mutex;              // A mutex for accessing the variable exiting 
    pthread_mutex_t buffer_mutex;               // A mutex for accessing the global queue (the buffer) 
    pthread_mutex_t jobs_running_mutex;         // A mutex for accessing the global variable jobs_currently_running 
    pthread_mutex_t concurrency_mutex;          // A mutex for accessing the global variable concurrencyLevel 
    pthread_mutex_t jobID_mutex;                // A mutex for accessing the global variable jobID_as_num 
    pthread_mutex_t buf_not_empty_mutex;        // A mutex for accessing the buf_not_empty variable 
    pthread_mutex_t buf_not_full_mutex;         // A mutex for accessing the buf_not_full variable 
    pthread_cond_t buf_not_empty_cond;          // A condition variable for when the buffer is not empty (has at least one job)
    pthread_cond_t buf_not_full_cond;           // A condition variable for when the buffer is not full (has space for at least one job)
} Shared_Data;

// A global struct for the shared data 
Shared_Data* shared_data;

// Gloabal variable representing the cocnurrency 
extern int concurrencyLevel;

// Gloabal variable representing the jobID number so that each process has a unique jobID (starting from 1) 
extern int jobID_as_num;

// Gloabal variable representing the number of jobs that are currently running 
extern int jobs_currently_running;

