#include "Queue.h"

Queue* queue_create(int max_size) {

    // Allocate memory for the Queue
    Queue* q = malloc(sizeof(Queue));

    // If the allocation failed exit with error code
    if(q == NULL) {
        perror("Queue allocation failed\n");
        exit(1);
    }

    // Initialize the front and rear pointers of the queue to NULL
    q->front = NULL;
    q->rear = NULL;
    q->size = 0;
    q->max_size = max_size;

    return q;
   
}

bool is_queue_empty(Queue* q) {
    
    // If size is 0 return true otherwise false
    return !q->size;

}

bool is_queue_full(Queue* q) {

    // If max size is equal to current size the queue is full
    if (q->max_size == q->size) 
        return true;
    return false;
}

int queue_insert(Queue* q, Job_Info* info) {

    // If the queue has reached it's maximum capacity return -1
    if (q->size == q->max_size) {
        return -1;
    }

    // Allocate a new queue node
    Queue_Node* new_node = malloc(sizeof(Queue_Node));

    // If the allocation failed exit with error code
    if (new_node == NULL) {
        perror("Queue node allocation failed\n");
        exit(1);
    }

    // Insert the triplet to the new node
    new_node->job_info = info;

    // TESTING
    // printf("INSERTING [<%s,%s>] \n", new_node->job_info->job,new_node->job_info->jobID);


    // If the queue is empty, new node becomes both front and rear
    if (q->size == 0) {

        q->front = new_node;
        q->rear = new_node;

        // Since it's the first node in the queue it has no next node
        new_node->next = NULL;
    } 
    else {

        // New node's next node is the rear node of the queue
        new_node->next = q->rear;

        // New node becomes the rear node's previous
        q->rear->previous = new_node;

        // New node becomes the rear of the queue
        q->rear = new_node;
    }
   
    // New node's previous node is NULL because it's the last node in the queue (regardless if the queue was empty or not initially)
    new_node->previous = NULL;

    // The size of the queue is increased by 1
    q->size += 1;

    return 0;
}


Job_Info* queue_remove(Queue* q) {

    // If queue is empty exit with error code
    if (q->size == 0) {
        perror("Attempted to remove from empty queue \n");
        return NULL;
    }
    
    // Store the node that is to be removed in a temp node
    Queue_Node* temp_node = q->front;
    Job_Info* removed_job_info = temp_node->job_info;

    // FOR TESTING
    // printf("REMOVING [<%s,%s>] \n", temp_node->job_info->job,temp_node->job_info->jobID);

    // The second element of the queue becomes the new front
    q->front = temp_node->previous;

    // If the queue is not empty, update the next pointer of the new front node
    if (q->front != NULL) {
        q->front->next = NULL;
    }
    else {
    // If the queue is empty now, rear should be NULL as well
        q->rear = NULL;
    }
    // Reduce the size of the queue by 1
    q->size -= 1;

    free(temp_node);
    
    // Return the info of the removed job
    return removed_job_info;
}

int queue_remove_ID(char* jobID, Queue* q) {

    int job_pos = queue_find_ID(jobID, q);

    // If there's not a job with the given jobID in the queue, or the queue is empty return -1
    if ((job_pos == -1) || (q->size == 0)) {
        return -1;
    }

    Queue_Node* node = queue_get_node(q, job_pos);
    Queue_Node* previous_node = NULL;
    Queue_Node* next_node = NULL;

    // If node is not the rear element in the queue, get it's next node
    if (node != q->rear) {
        next_node = node->next;
    }

    // If node is not the front element in the queue, get its previous node
    if (node != q->front) {
        previous_node = node->previous;
    }

    // If node is the rear element in the queue, its next node becomes the new rear
    if (node == q->rear) {
        q->rear = next_node;
    }

    // If node is the front element in the queue, its previous node becomes the new front
    if (node == q->front) {
        q->front = previous_node;
    }

    // Connect the previous and the next node, ignoring the one in the middle
    if (previous_node != NULL) {
        previous_node->next = next_node;
    }
    if (next_node != NULL) {
        next_node->previous = previous_node;
    }

    destroy_node(node);

    // Decrease the size of the queue
    q->size--;

    // If everything worked return 0
    return 0; 
}


int queue_find_ID(char* jobID, Queue* q) {

    // Initialize a node starting from the back of the queue
    Queue_Node* node = q->rear;

    int pos = q->size;

    // Go through all the nodes of the queue
    while (node != NULL) {

        // Check if the process with the given id is running
        if (strcmp(node->job_info->jobID, jobID) == 0) {
            
            // Return the job's position in the queue
            return pos;

        }
        pos --;

        // Go to the next node
        node = node->next;
    }

    // If the job was not found return -1
    return -1;
}

Queue_Node* queue_get_node(Queue* q, int pos) {

    // Initialize a node starting from the front of the queue
    Queue_Node* node = q->front;

    // Go through all the nodes of the queue
    for (int i = 1; i <= q->size; i++) {

        if (i == pos) {
            return node;
        }

        // Go to the previus node
        node = node->previous;
    }

    // If there's not a job in position pos return NULL
    return NULL;
}


void print_queue(Queue* q) {

    // Initiallize a node startng from the back of the queue
    Queue_Node* node = q->rear;

    // If the queue is empty return
    if (q->size == 0){
        printf("Queue is empty\n");
        return;
    }
    printf("Queue: ");

    while (node != NULL) {
        // Print all the queue elements in the same line
        printf("[<%s,%s>] ", node->job_info->job,node->job_info->jobID);
        node = node->next;
    }

    printf("\n");
}


void queue_destroy(Queue* q) {

    // Initialize a node
    Queue_Node* node = q->front;
    Queue_Node* temp;

    // Free each node of the queue
    while (node != NULL) {

        temp = node;
        node = node->previous;

        destroy_node(temp);
    }
    
    // Free the queue 
    if (q != NULL) {
        free(q);
    }

}

void destroy_node(Queue_Node* node) {

    // Destroy the node's info
    destroy_job_info(node->job_info);

    if (node != NULL) {
        free(node);
    }

}

void destroy_job_info(Job_Info* job_info) {

    if (job_info->jobID != NULL) {
        free(job_info->jobID);
    }
    if (job_info->job != NULL) {
        free(job_info->job); 
    }

    for (int i = 0; i < job_info->num_of_parameters; i++) {

        if (job_info->parameters[i] != NULL) {

            free(job_info->parameters[i]);
        }
    }
    if (job_info->parameters != NULL) {
        free(job_info->parameters);
    }
    if (job_info != NULL) {
        free(job_info);
    }

}

// // FOR TESTING //
// int main(void) {

//     // Create a queue
//     Queue* q = queue_create(5);

//     printf("Queue size is %d\n", q->size);
//     printf("The queue is empty: %s\n", is_queue_empty(q) ? "true" : "false");

//     char* name = "name";
//     char* job = "Job_";
//     char jobID_as_string[20]; 

//     Job_Info* job_info;

//     printf("\n");

//     // Insert 4 elements in the queue
//     for (int i = 1; i < 5 ; i ++) {

//         // Allocate memory for new job_info for each iteration
//         job_info = malloc(sizeof(Job_Info));

//         if (job_info == NULL) {
//             perror("Memory allocation failed.");
//             exit(EXIT_FAILURE);
//         }

//         int job_id = i;
//         sprintf(jobID_as_string, "%s%d", job, job_id);
        
//         job_info->job = name;
//         job_info->jobID = strdup(jobID_as_string); 

//         queue_insert(q, job_info);
//         printf("Queue size is %d\n", q->size);
//         printf("The queue is empty: %s\n", is_queue_empty(q) ? "true" : "false");
//         print_queue(q);

//         printf("\n");
//     }

//     printf("Found Job_2: %s\n", queue_find_ID("Job_2",q)== -1 ? "false" : "true");
//     printf("Found Job2: %s\n", queue_find_ID("Job2",q) == -1 ? "false" : "true");

//     printf("Job with id Job_3 is in pos: %d\n",queue_find_ID("Job_3",q));

//     printf("\n");

//     // Remove Job_2 from the queue
//     int result = queue_remove_ID("Job_2", q);
//     if (result == -1) {
//         printf("Job_2 not found in the queue.\n");
//     } else {
//         printf("Job_2 removed successfully from the queue.\n");
//     }
//     printf("Queue size is %d\n", q->size);
//     printf("The queue is empty: %s\n", is_queue_empty(q) ? "true" : "false");
//     print_queue(q);
//     printf("\n");

//     // Remove a non-existing job from the queue
//     printf("Removed Job_51: %s\n", queue_find_ID("Job_51",q)== -1 ? "false" : "true");

//     printf("Queue size is %d\n", q->size);
//     printf("The queue is empty: %s\n", is_queue_empty(q) ? "true" : "false");
//     print_queue(q);
//     printf("\n");

//     // Remove all elements from the queue one by one
//     while (!is_queue_empty(q)) {
//         queue_remove(q);
//         printf("Queue size is %d\n",q->size);
//         printf("The queue is empty: %s\n", is_queue_empty(q) ? "true" : "false");
//         print_queue(q);
//         printf("\n");
//     }
    
//     // Destroy the queue
//     queue_destroy(q);

//     return 0;
// }
