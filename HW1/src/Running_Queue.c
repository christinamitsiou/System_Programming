#include "Running_Queue.h"

Running_Queue* r_queue_create(void) {

    // Allocate memory for the Queue
    Running_Queue* q = malloc(sizeof(Running_Queue));

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

bool is_r_queue_empty(Running_Queue* q) {
    
    // If size is 0 return true otherwise false
    return !q->size;

}

void r_queue_insert(Running_Queue* q, Running_Job_Info* info) {

    // Allocate a new queue node
    Running_Queue_Node* new_node = malloc(sizeof(Running_Queue_Node));

    // If the allocation failed exit with error code
    if(new_node == NULL) {
        perror("Queue node allocation failed\n");
        exit(1);
    }

    // Insert the triplet to the new node
    new_node->job_info = info;
    // printf("INSERTING [<%s,%s,%d,%d>] \n", new_node->job_info->job,new_node->job_info->jobID,new_node->job_info->queuePosition,new_node->job_info->pid);

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


Running_Job_Info* r_queue_remove(Running_Queue* q) {

    // If queue is empty exit with error code
    if (q->size == 0) {
        perror("Attempted to remove from empty queue \n");
        exit(1);
    }
    
    // Store the node that is to be removed in a temp node
    Running_Queue_Node* temp_node = q->front;
    Running_Job_Info* removed_job_info = temp_node->job_info;

    // FOR TESTING
    // printf("REMOVING [<%s,%s,%d,%d>] \n", temp_node->job_info->job,temp_node->job_info->jobID,temp_node->job_info->queuePosition,temp_node->job_info->pid);

    // Free the memory allocated for the job that is to be removed
    free(q->front->job_info->jobID);
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
    Running_Queue_Node* node = q->rear;

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

Running_Job_Info* r_queue_remove_ID(char* jobID, Running_Queue* q) {

    int job_pos = r_queue_find_ID(jobID, q);

    // If there's not a job with the given jobID in the queue, or the queue is empty return -1
    if ((job_pos == -1) || (q->size == 0)) {
        return NULL;
    }

    Running_Queue_Node* node = r_queue_get_node(q, job_pos);
    Running_Queue_Node* previous_node = NULL;
    Running_Queue_Node* next_node = NULL;

    Running_Job_Info* removed_job_info = node->job_info;

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
    Running_Queue_Node* updated_pos_node = previous_node;
    // Go through all the nodes of the queue
    while (updated_pos_node != NULL) {

        // Update the queue position for all remaining elements (if they exist)
        updated_pos_node->job_info->queuePosition = updated_pos_node->job_info->queuePosition - 1;

        // Go to the next node
        updated_pos_node = node->previous;
    }

    // Free the job info
    free(node->job_info->jobID);
    free(node->job_info);

    // free to updated_pos_node??

    // Free the node
    free(node);

    // Decrease the size of the queue
    q->size--;

    // If everything worked return the removed job info
    return removed_job_info; 
}

Running_Job_Info* r_queue_remove_pid(pid_t pid, Running_Queue* q) {

    int job_pos = r_queue_find_pid(pid, q);

    // If there's not a job with the given jobID in the queue, or the queue is empty return -1
    if ((job_pos == -1) || (q->size == 0)) {
        return NULL;
    }

    Running_Queue_Node* node = r_queue_get_node(q, job_pos);
    Running_Queue_Node* previous_node = NULL;
    Running_Queue_Node* next_node = NULL;

    Running_Job_Info* removed_job_info = node->job_info;

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
    Running_Queue_Node* updated_pos_node = previous_node;
    // Go through all the nodes of the queue
    while (updated_pos_node != NULL) {

        // Update the queue position for all remaining elements (if they exist)
        updated_pos_node->job_info->queuePosition = updated_pos_node->job_info->queuePosition - 1;

        // Go to the next node
        updated_pos_node = node->previous;
    }

    // Free the job info
    free(node->job_info->jobID);
    free(node->job_info);

    // Free the node
    free(node);

    // Decrease the size of the queue
    q->size--;

    // If everything worked return the removed job info
    return removed_job_info; 
}

int r_queue_find_ID(char* jobID, Running_Queue* q) {

    // Initialize a node starting from the back of the queue
    Running_Queue_Node* node = q->rear;

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

int r_queue_find_pid(pid_t pid, Running_Queue* q) {

    // Initialize a node starting from the back of the queue
    Running_Queue_Node* node = q->rear;

    int pos = q->size;

    // Go through all the nodes of the queue
    while (node != NULL) {

        // Check if the process with the given pid is running
        if (pid == node->job_info->pid) {
            
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


Running_Queue_Node* r_queue_get_node(Running_Queue* q, int pos) {

    // Initialize a node starting from the front of the queue
    Running_Queue_Node* node = q->front;

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


void print_r_queue(Running_Queue* q) {

    // Initiallize a node startng from the back of the queue
    Running_Queue_Node* node = q->rear;

    if (q->size == 0){
        printf("Queue is empty\n");
        return;
    }
    printf("Queue: ");

    while (node != NULL) {
        // Print all the queue elements in the same line
        printf("[<%s,%s,%d,%d>] ", node->job_info->job,node->job_info->jobID,node->job_info->queuePosition,node->job_info->pid);
        node = node->next;
    }

    printf("\n");
}

void r_queue_destroy(Running_Queue* q) {

    // Initialize a node
    Running_Queue_Node* node = q->front;
    Running_Queue_Node* temp;

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
char* create_job_id(int jobID_as_num) {

    // Allocate memory for the job id as string (assuming it won't have more than 30 characters)
    char jobID_as_string[30]; 
    char* job = "Job_";
    
    // Combine the job variable and the specific job id to create the unique jobID as a string
    sprintf(jobID_as_string, "%s%d", job, jobID_as_num);
    
    // Duplicate the combined string
    char* job_name = strdup(jobID_as_string);

    return job_name;
}

// char* poll_running(Running_Queue* running_queue) {

//     int total_length = 0;

//     // Calculate total length required for the exit message
//     Running_Queue_Node* node = running_queue->rear;

//     // Before the string, have a message declaring this is the rear of the queue
//     total_length += strlen("QUEUE REAR --> ");

//     // Go through each node from beginning to end and calculate the length of each triplet
//     while (node != NULL) {
//         total_length += snprintf(NULL, 0, "<%s,%s,%d>, ", node->job_info->job, node->job_info->jobID, node->job_info->queuePosition);
//         node = node->next;
//     }

//     // After the string, have a message declaring this is the front of the queue
//     total_length += strlen(" <-- QUEUE FRONT");

//     // Allocate memory for exit message + 1 for the null terminator
//     char* exit_message = (char*)malloc(total_length + 1);

//     // If memory allocation failed, return NULL
//     if (exit_message == NULL) {
//         printf("Memory allocation failed!\n");
//         exit(EXIT_FAILURE);
//     }

//     // Keep a pointer to the beginning of the allocated memory
//     char* temp = exit_message;

//     // Start again from the last node of the queue
//     node = running_queue->rear;

//     // Add the message for declaring where the queue rear is to the front of the string
//     temp += sprintf(temp, "QUEUE REAR --> ");

//     // Generate the exit message
//     while (node != NULL) {
//         temp += sprintf(temp, "<%s,%s,%d>, ", node->job_info->job, node->job_info->jobID, node->job_info->queuePosition);
//         node = node->next;
//     }

//     // Remove the last comma and space
//     // if (total_length > strlen("QUEUE REAR --> ")) {
//     temp -= 2; // Move back to overwrite the last comma and space
//     *temp = '\0'; // Null-terminate the string
//     // }

//     // Add the message for declaring where the queue front is to the end of the string
//     sprintf(temp, " <--QUEUE FRONT");

//     return exit_message;
// }




// // FOR TESTING //
// int main(void) {

//     // Create a queue
//     Running_Queue* q = r_queue_create();

//     printf("Queue size is %d\n", q->size);
//     printf("The queue is empty: %s\n", is_r_queue_empty(q) ? "true" : "false");

//     char* name = "name";
//     char* job = "Job_";
//     char jobID_as_string[20];
//     pid_t pid = 0; 

//     Running_Job_Info* job_info;

//     printf("\n");

//     // Insert 4 elements in the queue
//     for (int i = 1; i < 5 ; i ++) {

//         // Allocate memory for new job_info for each iteration
//         job_info = malloc(sizeof(Running_Job_Info));

//         if (job_info == NULL) {
//             perror("Memory allocation failed.");
//             exit(EXIT_FAILURE);
//         }

//         int job_id = i;
//         sprintf(jobID_as_string, "%s%d", job, job_id);
        
//         job_info->job = name;
//         job_info->jobID = strdup(jobID_as_string); 
//         job_info->queuePosition = q->size + 1;
//         job_info->pid = pid;

//         r_queue_insert(q, job_info);
//         printf("Queue size is %d\n", q->size);
//         printf("The queue is empty: %s\n", is_r_queue_empty(q) ? "true" : "false");
//         print_r_queue(q);

//         printf("\n");

//         pid = pid + 10;

//     }

//     printf("Found Job_2: %s\n", r_queue_find_ID("Job_2",q)== -1 ? "false" : "true");
//     printf("Found Job2: %s\n", r_queue_find_ID("Job2",q) == -1 ? "false" : "true");

//     printf("Job with id Job_3 is in pos: %d\n", r_queue_find_ID("Job_3",q));

//     printf("\n");

//     // For testing poll_running
//     char* exit_message = poll_running(q);
//     printf("String is: %s\n",exit_message);
//     free(exit_message);

//     // Remove Job_2 from the queue
//     int result = r_queue_remove_ID("Job_2", q);
//     if (result == -1) {
//         printf("Job_2 not found in the queue.\n");
//     } else {
//         printf("Job_2 removed successfully from the queue.\n");
//     }
//     printf("Queue size is %d\n", q->size);
//     printf("The queue is empty: %s\n", is_r_queue_empty(q) ? "true" : "false");
//     print_r_queue(q);
//     printf("\n");

//     // Remove a non-existing job from the queue
//     printf("Removed Job_51: %s\n", r_queue_find_ID("Job_51",q)== -1 ? "false" : "true");

//     printf("Queue size is %d\n", q->size);
//     printf("The queue is empty: %s\n", is_r_queue_empty(q) ? "true" : "false");
//     print_r_queue(q);
//     printf("\n");

//     // Remove all elements from the queue one by one
//     while (!is_r_queue_empty(q)) {
//         r_queue_remove(q);
//         printf("Queue size is %d\n",q->size);
//         printf("The queue is empty: %s\n", is_r_queue_empty(q) ? "true" : "false");
//         print_r_queue(q);
//         printf("\n");
//     }
    
//     // Destroy the queue
//     r_queue_destroy(q);

//     return 0;
// }
