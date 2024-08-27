#pragma once 

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <string.h>
#include <signal.h>
#include <stdbool.h>

// Declare the global variables
 int jobID_as_num;
 int Concurrency;

// Define the pipes' names
#define JC_TO_JES_PIPE "jobCommander_to_jobExecutorServer"
#define JES_TO_JC_PIPE "jobExecutorServer_to_jobCommander"

// Define the file name
#define FILE_NAME "jobExecutorServer.txt"

