#include "Queue.h"
#include "header.h"

// Helping functions for JobExecutorServer

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// A function that uses the gloabal variable jobID_as_num and to make a unique jobID string
// It also increases the jobID_as_num by 1
char* convert_jobID_to_string();

// A function that creates the exit message after a job has been submitted in the buffer
char* format_issued_job_info(Job_Info* job_info);

// A function that creates the exit message after a new concurrency has been set
char* format_concurrency_set(int concurrency);

// A function that creates the exit message after a job has been found and removed from the buffer
char* format_removed_jobID(char* jobID);

// A function that creates the exit message after a job has not been found in the buffer, 
// and therefore cannot be removed from it
char* format_not_found_jobID(char* jobID);

// A function that reads the output file and returns a formatted version of it to be sent to the client
char* format_file_output(int output_file_desc,char* jobID);

// A function that creates the exit message for the poll command
char* format_poll_output(Queue* buffer);

// A function to set the Concurrency to a new value
// Returns 0 if it was successfull and -1 otherwise
int set_concurrency(int new_concurrency);

// A function that removes the job with jobID from the buffer queue
// Returns 0 if job was removed succesfully and -1 if it failed
int stop_job(char* jobID, Queue* buffer);

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
