#include "Waiting_Queue.h"

Waiting_Queue* w_queue_create(void) {

    // Allocate memory for the Queue
    Waiting_Queue* q = malloc(sizeof(Waiting_Queue));

    // If the allocation failed exit with error code
    if(q == NULL) {
        perror("Queue allocation failed\n");
        exit(1);
    }

    // Initialize the front and rear pointers of the queue to NULL
    q->front = NULL;
    q->rear = NULL;
    q->size = 0;

    return q;
   
}

bool is_w_queue_empty(Waiting_Queue* q) {
    
    // If size is 0 return true otherwise false
    return !q->size;

}

void w_queue_insert(Waiting_Queue* q, Waiting_Job_Info* info) {

    // Allocate a new queue node
    Waiting_Queue_Node* new_node = malloc(sizeof(Waiting_Queue_Node));

    // If the allocation failed exit with error code
    if(new_node == NULL) {
        perror("Queue node allocation failed\n");
        exit(1);
    }

    // Insert the triplet to the new node
    new_node->job_info = info;
    // new_node->pid = pid;
    printf("INSERTING [<%s,%s,%d>] \n", new_node->job_info->job,new_node->job_info->jobID,new_node->job_info->queuePosition);


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
}


Waiting_Job_Info* w_queue_remove(Waiting_Queue* q) {

    // If queue is empty exit with error code
    if (q->size == 0) {
        perror("Attempted to remove from empty queue \n");
        exit(1);
    }
    
    // Store the node that is to be removed in a temp node
    Waiting_Queue_Node* temp_node = q->front;
    Waiting_Job_Info* removed_job_info = temp_node->job_info;

    // FOR TESTING
    printf("REMOVING [<%s,%s,%d>] \n", temp_node->job_info->job,temp_node->job_info->jobID,temp_node->job_info->queuePosition);

    // Free the memory allocated for the job that is to be removed
    free(q->front->job_info->jobID);
    for (int i = 0; i < q->front->job_info->num_of_parameters; i++) {
        free(q->front->job_info->parameters[i]);
    }
    free(q->front->job_info);

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

    // Initiallize a node starting from the back of the queue
    Waiting_Queue_Node* node = q->rear;

    // Go through all the nodes of the queue
    while (node != NULL) {

        // Update the queue position for all remaining elements (if they exist)
        node->job_info->queuePosition = node->job_info->queuePosition - 1;

        // Go to the next node
        node = node->next;
    }

    // Free the node that stores the front of the old queue
    free(temp_node);
    
    // Return the info of the removed job
    return removed_job_info;
}

int w_queue_remove_ID(char* jobID, Waiting_Queue* q) {

    int job_pos = w_queue_find_ID(jobID, q);

    // If there's not a job with the given jobID in the queue, or the queue is empty return -1
    if ((job_pos == -1) || (q->size == 0)) {
        return -1;
    }

    Waiting_Queue_Node* node = w_queue_get_node(q, job_pos);
    Waiting_Queue_Node* previous_node = NULL;
    Waiting_Queue_Node* next_node = NULL;

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

    // All the nodes starting from the previous of the node that got removed get an updated position
    Waiting_Queue_Node* updated_pos_node = previous_node;

    // Go through all the nodes of the queue
    while (updated_pos_node != NULL) {

        // Update the queue position for all remaining elements (if they exist)
        updated_pos_node->job_info->queuePosition = updated_pos_node->job_info->queuePosition - 1;

        // Go to the next node
        updated_pos_node = node->previous;
    }

    // Free the job info
    free(node->job_info->jobID);
    for (int i = 0; i < node->job_info->num_of_parameters; i++) {
        free(node->job_info->parameters[i]);
    }    free(node->job_info);
    
    // Free the node
    free(node);

    // Decrease the size of the queue
    q->size--;

    // If everything worked return 0
    return 0; 
}


int w_queue_find_ID(char* jobID, Waiting_Queue* q) {

    // Initialize a node starting from the back of the queue
    Waiting_Queue_Node* node = q->rear;

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

Waiting_Queue_Node* w_queue_get_node(Waiting_Queue* q, int pos) {

    // Initialize a node starting from the front of the queue
    Waiting_Queue_Node* node = q->front;

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


void print_w_queue(Waiting_Queue* q) {

    // Initiallize a node startng from the back of the queue
    Waiting_Queue_Node* node = q->rear;

    if (q->size == 0){
        printf("Queue is empty\n");
        return;
    }
    printf("Queue: ");

    while (node != NULL) {
        // Print all the queue elements in the same line
        printf("[<%s,%s,%d>] ", node->job_info->job,node->job_info->jobID,node->job_info->queuePosition);
        node = node->next;
    }

    printf("\n");
}

void w_queue_destroy(Waiting_Queue* q) {

    // Initialize a node
    Waiting_Queue_Node* node = q->front;
    Waiting_Queue_Node* temp;

    // Free each node of the queue
    while (node != NULL) {

        temp = node;
        node = node->previous;

        // Free the job info
        free(temp->job_info->jobID);
        free(temp->job_info);

        free(temp);
    }
    
    // Free the queue 
    free(q);
}



// // FOR TESTING //
// int main(void) {

//     // Create a queue
//     Waiting_Queue* q = w_queue_create();

//     printf("Queue size is %d\n", q->size);
//     printf("The queue is empty: %s\n", is_w_queue_empty(q) ? "true" : "false");

//     char* name = "name";
//     char* job = "Job_";
//     char jobID_as_string[20]; 

//     Waiting_Job_Info* job_info;

//     printf("\n");

//     // Insert 4 elements in the queue
//     for (int i = 1; i < 5 ; i ++) {

//         // Allocate memory for new job_info for each iteration
//         job_info = malloc(sizeof(Waiting_Job_Info));

//         if (job_info == NULL) {
//             perror("Memory allocation failed.");
//             exit(EXIT_FAILURE);
//         }

//         int job_id = i;
//         sprintf(jobID_as_string, "%s%d", job, job_id);
        
//         job_info->job = name;
//         job_info->jobID = strdup(jobID_as_string); 
//         job_info->queuePosition = q->size + 1;

//         w_queue_insert(q, job_info);
//         printf("Queue size is %d\n", q->size);
//         printf("The queue is empty: %s\n", is_w_queue_empty(q) ? "true" : "false");
//         print_w_queue(q);

//         printf("\n");
//     }

//     printf("Found Job_2: %s\n", w_queue_find_ID("Job_2",q)== -1 ? "false" : "true");
//     printf("Found Job2: %s\n", w_queue_find_ID("Job2",q) == -1 ? "false" : "true");

//     printf("Job with id Job_3 is in pos: %d\n",w_queue_find_ID("Job_3",q));

//     printf("\n");

//     // Remove Job_2 from the queue
//     int result = w_queue_remove_ID("Job_2", q);
//     if (result == -1) {
//         printf("Job_2 not found in the queue.\n");
//     } else {
//         printf("Job_2 removed successfully from the queue.\n");
//     }
//     printf("Queue size is %d\n", q->size);
//     printf("The queue is empty: %s\n", is_w_queue_empty(q) ? "true" : "false");
//     print_w_queue(q);
//     printf("\n");

//     // Remove a non-existing job from the queue
//     printf("Removed Job_51: %s\n", w_queue_find_ID("Job_51",q)== -1 ? "false" : "true");

//     printf("Queue size is %d\n", q->size);
//     printf("The queue is empty: %s\n", is_w_queue_empty(q) ? "true" : "false");
//     print_w_queue(q);
//     printf("\n");

//     // Remove all elements from the queue one by one
//     while (!is_w_queue_empty(q)) {
//         w_queue_remove(q);
//         printf("Queue size is %d\n",q->size);
//         printf("The queue is empty: %s\n", is_w_queue_empty(q) ? "true" : "false");
//         print_w_queue(q);
//         printf("\n");
//     }
    
//     // Destroy the queue
//     w_queue_destroy(q);

//     return 0;
// }
