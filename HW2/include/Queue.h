#pragma once

#include "header.h"

typedef struct queue Queue;
typedef struct q_node Queue_Node;
typedef struct j_info Job_Info;

// A queue struct
struct queue {
    Queue_Node* front;         // A pointer to the front of the Queue
    Queue_Node* rear;          // A pointer to the rear of the Queue
    int size;                  // The number of elements currently in the queue
    int max_size;              // The max number of elements allowed in the queue
};

// A queue node 
struct q_node {
    Job_Info* job_info;         // A pointer to a struct containing info about the node
    Queue_Node* next;           // A pointer to the next node of the queue
    Queue_Node* previous;       // A pointer to the previous node of the queue
};

// A struct representing the triplet <jobID,job,clientSocket> as well as an extra string of arguments
// representing the parameters given for each job in the queue
struct j_info {
    char* jobID;             // The job ID of the job in the format job_XX
    char* job;               // The name of the job
    int clientSocket;        // The fle descriptor of the socket of the client
    int num_of_parameters;   // The size of the parameters array
    char** parameters;       // The rest of the parameters of the job (with the name)
};

// The functions that handle the buffer-queue

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// A function that creates an empty queue struct that has a capacity of max_size elements
Queue* queue_create(int max_size);

// A function that returns true if the queue given is empty and false otherwise
bool is_queue_empty(Queue* q);

// A function that returns true if the queue given is full and false otherwise
bool is_queue_full(Queue* q);

// A function that inserts a job_info struct at the end of the queue
// If it is succesfully inserted the function returns 0 otherwise -1
int queue_insert(Queue* q, Job_Info* info);

// A function that removes and returns the triplet in the front of the queue
// If it is succesfully removed the function returns a pointer to the removed job's info otherwise NULL
Job_Info* queue_remove(Queue* q);

// A function that removes a job with jobID from the queue
// If it succesfully found and removed the job it returns 0 otherwise -1
int queue_remove_ID(char* jobID, Queue* q);

// A function that searches for a job with the given jobID in the queue 
// If it exists it returns it's position in the queue otherwise -1
int queue_find_ID(char* jobID, Queue* q);

// A function that returns the node in position pos of the queue
Queue_Node* queue_get_node(Queue* q, int pos);

// A function that prints the elements of the queue
void print_queue(Queue* q);

// A function that deallocates all the memory for a queue and destroys it
void queue_destroy(Queue* q);

// A function that deallocates all the memory for a node and destroys it
void destroy_node(Queue_Node* node);

// A function that deallocates all the memory for a job_info struct and destroys it
void destroy_job_info(Job_Info* job_info);

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
