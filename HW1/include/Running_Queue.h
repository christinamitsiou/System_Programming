#include "header.h"

typedef struct r_queue Running_Queue;
typedef struct r_q_node Running_Queue_Node;
typedef struct r_j_info Running_Job_Info;

// A queue struct
struct r_queue {
    Running_Queue_Node* front;         // A pointer to the front of the queue
    Running_Queue_Node* rear;          // A pointer to the rear of the queue
    int size;                          // The number of elements in the queue
};

// A queue node for a running queue
struct r_q_node {
    Running_Job_Info* job_info;         // A pointer to a struct containing info about the node
    Running_Queue_Node* next;           // A pointer to the next node of the running queue
    Running_Queue_Node* previous;       // A pointer to the previous node of the running queue
};


// A struct representing the quadruplet <jobID,job,queuePosition,pid> for the processes in the running queue
struct r_j_info {
    char* jobID;                        // The job ID of the job in the format job_XX
    char* job;                          // The name of the job 
    int queuePosition;                  // The position of the job in the queue
    pid_t pid;                          // The process id of the job
};


// A function that creates an empty queue struct
Running_Queue* r_queue_create(void);

// A function that returns true if the queue given is empty and false otherwise
bool is_r_queue_empty(Running_Queue* q);

// A function that inserts a new node containing the info provided at the end of the queue
void r_queue_insert(Running_Queue* q, Running_Job_Info* info);

// A function that removes the first node of the queue and returns the info of said node 
Running_Job_Info* r_queue_remove(Running_Queue* q);

// A function that removes a job with jobID from the queue
// If it succesfully found and removed the job it returns it's info otherwise NULL
Running_Job_Info* r_queue_remove_ID(char* jobID, Running_Queue* q);

// A function that removes a job with a specific pid from the queue
// If it succesfully found and removed the job it returns it's info otherwise NULL
Running_Job_Info* r_queue_remove_pid(pid_t pid, Running_Queue* q);

// A function that searches for a job with the given jobID in the queue 
// If it exists it returns it's position in the queue otherwise -1
int r_queue_find_ID(char* jobID, Running_Queue* q);

// A function that searches for a job with the given pid in the queue 
// If it exists it returns it's position in the queue otherwise -1
pid_t r_queue_find_pid(pid_t pid, Running_Queue* q);

// A function that returns the node in position pos of the queue
Running_Queue_Node* r_queue_get_node(Running_Queue* q, int pos);

// A function that prints the elements of the queue
void print_r_queue(Running_Queue* q);

// A function that deallocates all the memory for the queue and destroys it
void r_queue_destroy(Running_Queue* q);

