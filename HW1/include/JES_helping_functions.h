#include "Waiting_Queue.h"
#include "Running_Queue.h"
#include "header.h"

// Helping functions for JobExecutorServer

// Declare a global queue pointer for the waiting jobs
Waiting_Queue* waiting_queue;

// Declare a global queue pointer for the running jobs
Running_Queue* running_queue;

// A function that uses the gloabal variable jobID_as_num and to make a unique jobID string
// It also increases the jobID_as_num by 1
char* convert_jobID_to_string();

// A function that creates the exit signal after a job has been issued and inserted into the waiting queue
char* format_w_job_info(Waiting_Job_Info* job_info);

// A function that creates the exit signal after a job has been issued and inserted into the running queue
char* format_r_job_info(Running_Job_Info* job_info);

// A function to issue a job and insert it into the waiting queue
Waiting_Job_Info* issue_waiting_job(int argc,char* argv[], Waiting_Queue* waiting_queue);

// A function to issue a job and insert it into the running queue
// If the job was previously in the waiting queue, argv is NULL.
// If the job was never in the waiting queue, which means it didn't wait to get executed, waiting_info is NULL
Running_Job_Info* issue_running_job(Waiting_Job_Info* waiting_info,char* argv[], Running_Queue* running_queue);

// A function to set the Concurrency to a new value
// Returns 0 if it was successfull and -1 otherwise
int set_concurrency(int new_concurrency, Waiting_Queue* waiting_queue, Running_Queue* running_queue);

// A function that traverses the running queue and returns a string of information for every running process 
// The format of the string is this <job1,job,2>,<job6,job,1> etc
char* poll_running(Running_Queue* running_queue);

// A function that traverses the waiting queue and returns a string of information for every waiting process 
// The format of the string is this <job1,job,2>,<job6,job,1> etc
char* poll_waiting(Waiting_Queue* waiting_queue);

// A function that stops the process with jobID if it's running, or removes it from the waiting queue if not
// Returns 0 if job was removed from the running queue, 1 if it was removed from the waiting queue and -1 if it failed
int stop_job(char* jobID,Running_Queue* running_queue, Waiting_Queue* waiting_queue);

// A function that handles the signal that jobCommander sends to jobExecutorSever to alert him about a message
// being written to the pipe and all the actions that need to be taken depending on the command
void jc_to_jes_sig_handler(int sig, siginfo_t* sig_info, void* context);

// A function that receieves a signal of a child process that ended, removes said process from the running queue
// and starts executing the next process (job) in the waiting queue (if it exists)
void sigchild_handler(int sig);

