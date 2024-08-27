#include "JES_helping_functions.h"


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


char* convert_jobID_to_string() {

    // Lock the jobID mutex to access the global variable
    pthread_mutex_lock(&(shared_data->jobID_mutex));
    
    // Allocate memory for the job id as string (assuming it won't have more than 30 characters)
    char jobID_as_string[30];
    char* job = "Job_";

    // Combine the job variable and the global variable jobID_as_num to create the unique jobID as a string
    snprintf(jobID_as_string, sizeof(jobID_as_string), "%s%d", job, jobID_as_num);

    // Duplicate the combined string
    char* job_name = strdup(jobID_as_string);

    // If strdup failed return NULL
    if (job_name == NULL) {
        // Unlock the jobID mutex
        pthread_mutex_unlock(&(shared_data->jobID_mutex));
        return NULL;
    }

    // Increase the global variable
    jobID_as_num++;

    // Unlock the jobID mutex
    pthread_mutex_unlock(&(shared_data->jobID_mutex));
    
    return job_name;
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


char* format_issued_job_info(Job_Info* job_info) {

    // Calculate the size of the string
    int size = snprintf(NULL, 0, "JOB <%s, %s> SUBMITTED\n",job_info->jobID, job_info->job);
    
    char* job_info_to_string;

    // Allocate memory for the new string and add 1 for null terminator
    job_info_to_string = (char*)malloc(size + 1);

    // If allocation failed return NULL
    if (job_info_to_string == NULL) {
        perror("Failed to allocate memory");
        return NULL;
    }

    // Store the string 
    snprintf(job_info_to_string, size + 1, "JOB <%s, %s> SUBMITTED\n",job_info->jobID, job_info->job);
    
    return job_info_to_string;
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


char* format_concurrency_set(int concurrency) {

    // Calculate the size of the string
    int size = snprintf(NULL, 0, "CONCURRENCY SET AT %d\n",concurrency);
    
    char* job_info_to_string;

    // Allocate memory for the new string and add 1 for null terminator
    job_info_to_string = (char*)malloc(size + 1);

    // If allocation failed return NULL
    if (job_info_to_string == NULL) {
        perror("Failed to allocate memory");
        return NULL;
    }

    // Store the string 
    snprintf(job_info_to_string, size + 1, "CONCURRENCY SET AT %d\n",concurrency);
    
    return job_info_to_string;
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


char* format_removed_jobID(char* jobID) {

    // Calculate the size of the string
    int size = snprintf(NULL, 0, "JOB <%s> REMOVED\n",jobID);
    
    char* job_info_to_string;

    // Allocate memory for the new string and add 1 for null terminator
    job_info_to_string = (char*)malloc(size + 1);

    // If allocation failed return NULL
    if (job_info_to_string == NULL) {
        perror("Failed to allocate memory");
        return NULL;
    }

    // Store the string 
    snprintf(job_info_to_string, size + 1, "JOB <%s> REMOVED\n",jobID);
    
    return job_info_to_string;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


char* format_not_found_jobID(char* jobID) {

    // Calculate the size of the string
    int size = snprintf(NULL, 0, "JOB <%s> NOTFOUND\n",jobID);
    
    char* job_info_to_string;

    // Allocate memory for the new string and add 1 for null terminator
    job_info_to_string = (char*)malloc(size + 1);

    // If allocation failed return NULL
    if (job_info_to_string == NULL) {
        perror("Failed to allocate memory");
        return NULL;
    }

    // Store the string 
    snprintf(job_info_to_string, size + 1, "JOB <%s> NOTFOUND\n",jobID);
    
    return job_info_to_string;
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


char* format_file_output(int output_file_desc,char* jobID) {

    ssize_t bytes_read;

    // Allocate buffer dynamically
    char* buf = (char *)malloc(BUFSIZ);

    // If allocation failed exit with error code 
    if (buf == NULL) {           
        perror("Allocating memory for buffer failed");
        exit(EXIT_FAILURE);
    }

    // Store its original size to use if needed for reallocation of the buffer
    int size_of_buf = BUFSIZ; 
    
    // Read and the arguments from jobExecutorServer
    bytes_read = read(output_file_desc, buf, size_of_buf);

    // If reading from the socket failed exit with error code
    if (bytes_read == -1) { 
        perror("Reading from the server failed");
        exit(EXIT_FAILURE);
    }

    int shift_offset = 0;

    // Read the message until there's no more bytes to read
    while (bytes_read > 0) {

        // Shift the offset by the bytes read in the last while loop
        shift_offset += bytes_read;

        // If the offset is equal to/bigger than the size of the buffer, double the buffer
        if (shift_offset >= size_of_buf) { 

            // Double the buffer size
            size_of_buf *= 2;              

            // Reallocate memory for the buffer
            buf = (char*)realloc(buf, size_of_buf);

            // If allocation failed exit with error code
            if (buf == NULL) {    
                perror("Reallocating memory for buffer failed");
                exit(EXIT_FAILURE);
            }
        }
        
        // The next set of bytes to be read is stored in the buffer after the last round of bytes (using shift to get to that position)
        bytes_read = read(output_file_desc, buf + shift_offset, size_of_buf - shift_offset);
    }

    // If reading from the socket failed exit with error code
    if (bytes_read == -1) { 
        perror("Reading from the server failed");
        exit(EXIT_FAILURE);
    }

    // Ensure the buffer is null-terminated 
    buf[shift_offset] = '\0';

    // Allocate buffer for the formatted output
    int jobID_len = strlen(jobID);

    // Format strings for the beginning and the end of the output
    char* output_start_1 = "-----";
    char* output_start_2 = " output start-----\n\n";
    char* format_end_1 = "-----";
    char* format_end_2 = " output end-----\n";

    // Calculate the total length
    int total_len = shift_offset + strlen(output_start_1) + strlen(output_start_2) + strlen(format_end_1) + strlen(format_end_2) + jobID_len * 2 + 1;

    // Allocate memory for the output
    char* formatted_buf = (char*)malloc(total_len);

    // If allocation failed exit with error code
    if (formatted_buf == NULL) {
        perror("Allocating memory for formatted buffer failed");
        exit(EXIT_FAILURE);
    }

    // Format the output
    snprintf(formatted_buf, total_len, "-----%s output start-----\n\n%s\n-----%s output end-----\n", jobID, buf, jobID);

    // Ensure the formatted buffer is null-terminated
    formatted_buf[total_len - 1] = '\0';

    // Free the original buffer
    free(buf);

    return formatted_buf;
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


int set_concurrency(int new_concurrency) {

    // Lock the concurrency mutex to access the global variable
    pthread_mutex_lock(&(shared_data->concurrency_mutex));

    // If the new concurrency is an invalid number, return -1
    if (new_concurrency <= 0) {
        // Unlock the concurrency mutex 
        pthread_mutex_unlock(&(shared_data->concurrency_mutex));
        return -1;
    }

    // Update the global variable to the new value
    concurrencyLevel = new_concurrency;

    // Unlock the concurrency mutex 
    pthread_mutex_unlock(&(shared_data->concurrency_mutex));

    return 0;
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


int stop_job(char* jobID, Queue* buffer) {

    // Attempt to find the job in the buffer
    int pos = queue_find_ID(jobID, buffer);

    // If result is greater than 0, it means the job was found in the buffer
    if (pos > 0) {

        Queue_Node* node = queue_get_node(buffer,pos);

        // Sent an appropriate message to the client that issued the job
        char* exit_message = "JOB STOPPED FROM ANOTHER CLIENT";

        // If writing the exit message failed exit with error code
        if (write(node->job_info->clientSocket, exit_message, strlen(exit_message)) < 0) {
            fprintf(stderr,"Writing message to the client failed here\n");
            exit(EXIT_FAILURE);
        }

        // Close the client socket
        close(node->job_info->clientSocket);

        // Remove the job from the queue and get it's info
        int result = queue_remove_ID(jobID, buffer);
        
        // If result is 0 it means that removing the job from the queue was successfull
        if (result == 0) {

            // Return 0 to signal successful removal 
            return 0;

        }

    }
    
    // Return -1 to signal failure to find the job in the buffer
    return -1;
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


char* format_poll_output(Queue* buffer) {

    // If the queue is empty return an appropriate message
    if (is_queue_empty(buffer)) {

        char* exit_message = "No jobs in the buffer at the moment\n";

        return exit_message;
    }
    
    int total_length = 0;

    // Start from the front of the buffer-queue
    Queue_Node* node = buffer->front;

    // Go through all the nodes in the queue
    while (node != NULL) {

        // Calculate the length for each job's info 
        total_length += snprintf(NULL, 0, "<%s,%s>\n",node->job_info->jobID, node->job_info->job);

        // Go to the previous node
        node = node->previous;
    }

    // Allocate memory for the exit message + 1 for null terminator
    char* exit_message = (char*)malloc(total_length + 1);

    // If memory allocation failed, return NULL
    if (exit_message == NULL) {
        printf("Memory allocation failed!\n");
        return NULL;
    }

    // Keep a pointer to the beginning of the allocated memory
    char* temp = exit_message;

    // Start again from the first node of the queue
    node = buffer->front;

    // Go through all the nodes in the queue
    while (node != NULL) {

        // Generate the exit message for each node
        temp += sprintf(temp, "<%s,%s>\n",node->job_info->jobID, node->job_info->job);

        // Go to the previous node
        node = node->previous;
    }

    // Null-terminate the string
    *temp = '\0';

    return exit_message;
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

