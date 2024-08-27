#include "header.h"

typedef struct w_queue Waiting_Queue;
typedef struct w_q_node Waiting_Queue_Node;
typedef struct w_j_info Waiting_Job_Info;

// A queue struct
struct w_queue {
    Waiting_Queue_Node* front;         // A pointer to the front of the queue
    Waiting_Queue_Node* rear;          // A pointer to the rear of the queue
    int size;                          // The number of elements in the queue
};

// A queue node for a waiting queue
struct w_q_node {
    Waiting_Job_Info* job_info;         // A pointer to a struct containing info about the node
    Waiting_Queue_Node* next;           // A pointer to the next node of the waiting queue
    Waiting_Queue_Node* previous;       // A pointer to the previous node of the waiting queue
};

// A struct representing the triplet <jobID,job,queuePosition> as well as an integer representin the number of
// arguments and a string array representing the parameters given after the name for the processes in the waiting queue
struct w_j_info {
    char* jobID;             // The job ID of the job in the format job_XX
    char* job;               // The name of the job
    int queuePosition;       // The position of the job in the queue
    int num_of_parameters;   // The size of the parameters array
    char* parameters[];      // The rest of the parameters of the job (after the name) or NULL if they don't exist
};


// A function that creates an empty queue struct
Waiting_Queue* w_queue_create(void);

// A function that returns true if the queue given is empty and false otherwise
bool is_w_queue_empty(Waiting_Queue* q);

// A function that inserts a new node conatining the info provided at the end of the queue
void w_queue_insert(Waiting_Queue* q, Waiting_Job_Info* info);

// A function that removes the first node of the queue and returns the info of said node 
Waiting_Job_Info* w_queue_remove(Waiting_Queue* q);

// A function that removes a job with jobID from the queue
// If it succesfully found and removed the job it returns 0 otherwise -1
int w_queue_remove_ID(char* jobID, Waiting_Queue* q);

// A function that searches for a job with the given jobID in the queue 
// If it exists it returns it's position in the queue otherwise -1
int w_queue_find_ID(char* jobID, Waiting_Queue* q);

// A function that returns the node in position pos of the queue
Waiting_Queue_Node* w_queue_get_node(Waiting_Queue* q, int pos);

// A function that prints the elements of the queue
void print_w_queue(Waiting_Queue* q);

// A function that deallocates all the memory for the queue and destroys it
void w_queue_destroy(Waiting_Queue* q);

