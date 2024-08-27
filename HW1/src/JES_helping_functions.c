#include "JES_helping_functions.h"


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


char* convert_jobID_to_string() {
    
    // Allocate memory for the job id as string (assuming it won't have more than 30 characters)
    char jobID_as_string[30];
    char* job = "Job_";

    // Combine the job variable and the global variable jobID_as_num to create the unique jobID as a string
    snprintf(jobID_as_string, sizeof(jobID_as_string), "%s%d", job, jobID_as_num);

    // Duplicate the combined string
    char* job_name = strdup(jobID_as_string);

    // If strdup failed return NULL
    if (job_name == NULL) {
        return NULL;
    }

    // Increase the global variable
    jobID_as_num++;

    return job_name;
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


char* format_w_job_info(Waiting_Job_Info* job_info) {

    // Calculate the size of the string
    int size = snprintf(NULL, 0, "Issued job inserted into waiting queue: [%s,%s,%d]\n", job_info->job, job_info->jobID, job_info->queuePosition);
    
    char* job_info_to_string;

    // Allocate memory for the new string and add 1 for null terminator
    job_info_to_string = (char*)malloc(size + 1);

    // If allocation failed return NULL
    if (job_info_to_string == NULL) {
        perror("Failed to allocate memory");
        return NULL;
    }

    // Store the string 
    snprintf(job_info_to_string, size + 1, "Issued job inserted into waiting queue: [%s,%s,%d]\n", job_info->job, job_info->jobID, job_info->queuePosition);
    
    return job_info_to_string;
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


char* format_r_job_info(Running_Job_Info* job_info) {

    // Calculate the size of the string
    int size = snprintf(NULL, 0, "Issued job inserted into running queue: [<%s,%s,%d>]\n", job_info->job, job_info->jobID, job_info->queuePosition);
    
    char* job_info_to_string;
    
    // Allocate memory for the new string and add 1 for null terminator
    job_info_to_string = (char*)malloc(size + 1);

    // If allocation failed return NULL
    if (job_info_to_string == NULL) {
        perror("Failed to allocate memory");
        return NULL;
    }

    // Store the string 
    snprintf(job_info_to_string, size + 1, "Issued job inserted into running queue: [<%s,%s,%d>]\n", job_info->job, job_info->jobID, job_info->queuePosition);
    
    return job_info_to_string;
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


Running_Job_Info* issue_running_job(Waiting_Job_Info* waiting_info, char* argv[], Running_Queue* running_queue) {

    // Allocate memory for the new job's information
    Running_Job_Info* new_job_info = malloc(sizeof(Running_Job_Info));
    
    pid_t pid;

    // If the allocation failed exit with error code
    if(new_job_info == NULL) {
        perror("New job info allocation failed\n");
        exit(EXIT_FAILURE);
    }

    // If waiting_info is not NULL it means that the job was previously in the waiting queue 
    // Therefore it already has a name and jobID so we store them in the new job info
    if (waiting_info != NULL) {
        new_job_info->job = waiting_info->job;
        new_job_info->jobID = waiting_info->jobID;
    }
    // If waiting_info is NULL then the process didn't come from the waiting queue and therefore we create it's jobID on the spot
    else {
        
        // The name of the job is the first argument of the argv array
        new_job_info->job = strdup(argv[1]);

        // If strdup failed exit with error code
        if (new_job_info->job == NULL) {
            perror("Job name duplication failed\n");
            free(new_job_info);
            exit(EXIT_FAILURE);
        }

        // Get the new job's unique id 
        new_job_info->jobID = convert_jobID_to_string();

        // If creating the id failed exit with error code
        if (new_job_info->jobID == NULL) {
            perror("Job ID creation failed\n");
            free(new_job_info->job);
            free(new_job_info);
            exit(EXIT_FAILURE);
        }
    }

    // The job's position in the queue is the size of the queue + 1
    new_job_info->queuePosition = running_queue->size + 1;

    pid = fork();

    // If fork function failed exit with error code
    if (pid < 0) {
        perror("Fork failed");
        free(new_job_info->job);
        free(new_job_info->jobID);
        free(new_job_info);
        exit(EXIT_FAILURE);
    } 

    if (pid == 0) {
        
        // If waiting_info is not NULL it means that the job was previously in the waiting queue 
        // Therefore we use waiting_info->job (for the name) and waiting_info->parameters (for the rest of the parameters) to call execvp
        if (waiting_info != NULL) {

            // First argument in execvp is the name of the job, and the second one is the rest of the parameters
            execvp(waiting_info->job, waiting_info->parameters);

            // If execvp fails exit with error code
            perror("execvp failed");
            exit(EXIT_FAILURE);
        }
        // If waiting_info is NULL then the process didn't come from the waiting queue so we use argv to get the name and parameters
        else {
            // First argument in execvp is the name of the job, and the second one is the rest of the parameters
            execvp(argv[1], &argv[1]);

            // If execvp fails
            perror("execvp failed");
            exit(EXIT_FAILURE);
        }
    }
    
    // Store the job's pid
    new_job_info->pid = pid;

    // Insert the job into the running queue
    r_queue_insert(running_queue, new_job_info);

    return new_job_info;
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


Waiting_Job_Info* issue_waiting_job(int argc, char* argv[], Waiting_Queue* waiting_queue) {

    // Allocate memory for the struct and the array
    Waiting_Job_Info* new_job_info = malloc(sizeof(Waiting_Job_Info) + (argc - 1) * sizeof(char*));

    // Check if allocation was successful
    if (new_job_info == NULL) {
        perror("New job info allocation failed\n");
        exit(EXIT_FAILURE);
    }

    // Store the name of the job
    new_job_info->job = strdup(argv[1]);

    // If strdup failed exit with error code
    if (new_job_info->job == NULL) {
        perror("Job name duplication failed\n");
        free(new_job_info); 
        exit(EXIT_FAILURE);
    }

    // Store the parameters in the array
    for (int i = 0; i < argc - 1; i++) {

        new_job_info->parameters[i] = strdup(argv[i + 1]);

        // If strdup failed exit with error code
        if (new_job_info->parameters[i] == NULL) {

            perror("Parameter duplication failed\n");

            // Free previously the allocated memory for job_info
            free(new_job_info->job); 
            for (int j = 0; j < i; j++) {
                free(new_job_info->parameters[j]);
            }
            free(new_job_info); 
            exit(EXIT_FAILURE);
        }
    }

    new_job_info->num_of_parameters = argc - 1;

    // Get the new job's unique id 
    new_job_info->jobID = convert_jobID_to_string(); 

    // If creating the id failed exit with error code
    if (new_job_info->jobID == NULL) {

        perror("Job ID creation failed\n");

        // Free previously the allocated memory for job_info
        free(new_job_info->job);
        for (int i = 0; i < new_job_info->num_of_parameters; i++) {
            free(new_job_info->parameters[i]);
        } 
        free(new_job_info); 
        exit(EXIT_FAILURE);
    }

    // The job's position in the queue is the size of the queue + 1
    new_job_info->queuePosition = waiting_queue->size + 1;

    // Insert the new job into the Queue
    w_queue_insert(waiting_queue, new_job_info);

    // Return the struct containing the new job's info      
    return new_job_info;
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


int set_concurrency(int new_concurrency, Waiting_Queue* waiting_queue, Running_Queue* running_queue) {

    Waiting_Job_Info* waiting_job_info;
    Running_Job_Info* running_job_info;

    // If the new concurrency is an invalid number, return -1
    if (new_concurrency <= 0) {
        return -1;
    }

    // If the new concurrency is larger than the previous one, we issue as many jobs as the new concurrency allows
    if (new_concurrency > Concurrency) {

        // The number of jobs to start executing is the difference between the new and the old concurrency
        int jobs_to_be_issued_count = new_concurrency - Concurrency;

        for (int i = 0; i < jobs_to_be_issued_count; i++) {

            // If there are jobs in the waiting queue, issue them to start executing
            if (!is_w_queue_empty(waiting_queue)) {
                
                // Remove the job from the front of the waiting queue and store it's info
                waiting_job_info = w_queue_remove(waiting_queue);

                // Issue the waiting job to start running
                running_job_info = issue_running_job(waiting_job_info, NULL, running_queue);

                // If issuing the running job fails, exit with error code
                if (running_job_info == NULL) {
                    perror("Failed to issue a running job");
                    exit(EXIT_FAILURE);
                }
            }
            else {
                // If there are no more jobs in the waiting queue, exit the loop
                break;
            }
        }

    }

    // Update the global variable Concurrency to the new value
    Concurrency = new_concurrency;

    return 0;
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


int stop_job(char* jobID, Running_Queue* running_queue, Waiting_Queue* waiting_queue) {

    // Attempt to find the job in the running queue
    int result = r_queue_find_ID(jobID, running_queue);

    // If result is greater than 0, it means the job was found in the running queue
    if (result > 0) {

        // Remove the job from the queue and get its info
        Running_Job_Info* job_info = r_queue_remove_ID(jobID, running_queue);
        
        // If killing the process failed, print an error message and return -1
        if (kill(job_info->pid, SIGKILL) != 0) {
            perror("Error killing process");
            return -1;
        }

        // Free memory allocated for the job info
        free(job_info->job);
        free(job_info->jobID);
        free(job_info);

        // Return 0 to signal successful removal from the running queue
        return 0;
    }
    
    // If the job wasn't found in the running queue, search for it in the waiting queue
    result = w_queue_find_ID(jobID, waiting_queue);
        
    // If result is greater than 0, it means the job was found in the waiting queue
    if (result > 0) {

        // Remove the job from the queue
        result = w_queue_remove_ID(jobID, waiting_queue);

        // If removing the job was unsuccessful, return -1
        if (result == -1) {
            return -1;
        }
        
        // Return 1 to signal successful removal from the waiting queue
        return 1;
    }

    // Return -1 to signal failure to find the job in any of the queues
    return -1;
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


char* poll_running(Running_Queue* running_queue) {

    // If the running queue is empty return an appropriate message
    if (is_r_queue_empty(running_queue)) {
        char* exit_message = "No processes are running at the moment\n";
        return exit_message;
    }
    
    int total_length = 0;

    // Calculate total length required for the exit message
    Running_Queue_Node* node = running_queue->rear;

    // Add length for the first part of the string
    total_length += strlen("QUEUE REAR --> ");

    // Calculate the length for each triplet in the queue
    while (node != NULL) {
        total_length += snprintf(NULL, 0, "<%s,%s,%d>, ", node->job_info->job, node->job_info->jobID, node->job_info->queuePosition);
        node = node->next;
    }

    // Add length for the final part of the string
    total_length += strlen(" <--QUEUE FRONT");

    // Allocate memory for the exit message + 1 for null terminator
    char* exit_message = (char*)malloc(total_length + 1);

    // If memory allocation failed, return NULL
    if (exit_message == NULL) {
        printf("Memory allocation failed!\n");
        return NULL;
    }

    // Keep a pointer to the beginning of the allocated memory
    char* temp = exit_message;

    // Start again from the last node of the queue
    node = running_queue->rear;

    // Add the message for declaring where the queue rear is to the front of the string
    temp += sprintf(temp, "QUEUE REAR --> ");

    // Generate the exit message for each node
    while (node != NULL) {
        temp += sprintf(temp, "<%s,%s,%d>, ", node->job_info->job, node->job_info->jobID, node->job_info->queuePosition);
        node = node->next;
    }

    // Move back 2 characters to overwrite the last comma and space
    temp -= 2;

    // Add the message for declaring where the queue front is to the end of the string
    strcat(temp, " <--QUEUE FRONT");

    // Null-terminate the string
    *temp = '\0';

    return exit_message;
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


char* poll_waiting(Waiting_Queue* waiting_queue) {

    // If the waiting queue is empty return an appropriate message
    if (is_w_queue_empty(waiting_queue)) {
        char* exit_message = "No processes are waiting to be executed at the moment\n";
        return exit_message;
    }
    
    int total_length = 0;

    // Calculate total length required for the exit message
    Waiting_Queue_Node* node = waiting_queue->rear;

    // Add length for the initial part of the string
    total_length += strlen("QUEUE REAR --> ");

    // Calculate the length for each triplet in the queue
    while (node != NULL) {
        total_length += snprintf(NULL, 0, "<%s,%s,%d>, ", node->job_info->job, node->job_info->jobID, node->job_info->queuePosition);
        node = node->next;
    }

    // Add length for the final part of the string
    total_length += strlen(" <-- QUEUE FRONT");

    // Allocate memory for the exit message + 1 for null terminator
    char* exit_message = (char*)malloc(total_length + 1);

    // If memory allocation failed, return NULL
    if (exit_message == NULL) {
        printf("Memory allocation failed!\n");
        exit(EXIT_FAILURE);
    }

    // Keep a pointer to the beginning of the allocated memory
    char* temp = exit_message;

    // Start again from the last node of the queue
    node = waiting_queue->rear;

    // Add the message for declaring where the queue rear is to the front of the string
    temp += sprintf(temp, "QUEUE REAR --> ");

    // Generate the exit message for each node
    while (node != NULL) {
        temp += sprintf(temp, "<%s,%s,%d>, ", node->job_info->job, node->job_info->jobID, node->job_info->queuePosition);
        node = node->next;
    }

    // Move back 2 characters to overwrite the last comma and space
    temp -= 2;

    // Add the message for declaring where the queue front is to the end of the string
    strcat(temp, " <--QUEUE FRONT");

    // Null-terminate the string
    *temp = '\0';

    return exit_message;
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


void jc_to_jes_sig_handler(int sig, siginfo_t* sig_info, void* context) {

    // Initialize the pipes
    char* jc_to_jes_pipe = JC_TO_JES_PIPE;
    char* jes_to_jc_pipe = JES_TO_JC_PIPE;

    // Initialize the file descriptors
    int file_desc_write;
    int file_desc_read;

    char* exit_message;
    int argc;

    ssize_t bytes_read;

    ////////////////////////////////////////////////////////////////////////
    // Reading the input from jobCommander

    // Open the named pipe for reading from jobCommander
    file_desc_read = open(jc_to_jes_pipe, O_RDONLY);

    // If opening the pipe failed, exit with error code
    if (file_desc_read == -1) {
        perror("Opening the pipe JC_TO_JES failed");
        exit(EXIT_FAILURE);
    }

    // TESTING
    // printf("Named pipe opened for reading from jobCommander\n");

    // Allocate buffer dynamically
    char* buffer = (char*)malloc(BUFSIZ);

    // Store its original size to use if needed for reallocation of the buffer
    int size_of_buffer = BUFSIZ;

    // If allocation failed, exit with error code
    if (buffer == NULL) {
        perror("Allocating memory for buffer failed");
        exit(EXIT_FAILURE);
    }

    // Read the argument count from the named pipe
    bytes_read = read(file_desc_read, &argc, sizeof(argc));

    // If reading from the pipe failed, exit with error code
    if (bytes_read == -1) {
        perror("read");
        exit(EXIT_FAILURE);
    }

    // Read the arguments from jobCommander
    bytes_read = read(file_desc_read, buffer, size_of_buffer);

    int shift_offset = 0;

    // Read the message until there's no more bytes to read
    while (bytes_read > 0) {

        // Shift the offset by the bytes read in the last loop
        shift_offset += bytes_read;

        // If the offset is equal to or bigger than the size of the buffer, double the buffer
        if (shift_offset >= size_of_buffer) {

            // Double the buffer size
            size_of_buffer *= 2;

            // Reallocate memory for the buffer
            buffer = (char*)realloc(buffer, size_of_buffer);

            // If allocation failed, exit with error code
            if (buffer == NULL) {
                // Free the buffer
                free(buffer);
                perror("Reallocating memory for buffer failed");
                exit(EXIT_FAILURE);
            }
        }

        // The next set of bytes to be read is stored in the buffer after the last round of bytes (using shift to get to that position)
        bytes_read = read(file_desc_read, buffer + shift_offset, size_of_buffer - shift_offset);
    }

    // If reading from the pipe failed, exit with error code
    if (bytes_read == -1) {
        perror("Reading from the pipe failed");
        exit(EXIT_FAILURE);
    }

    // Close the named pipe for reading
    if (close(file_desc_read) == -1) {
        perror("close");
        exit(EXIT_FAILURE);
    }

    // Unlink the pipe
    unlink(jc_to_jes_pipe);

    //////////////////////////////////////////////////////////////////////////////////////
    // Tokenize the input so we can better process it

    // TESTING
    // printf("COMMAND RECEIVED IS:%s\n",buffer);

    // Start reading the arguments from buffer
    char* argument = strtok(buffer, " ");

    // Create an array of pointers to each argument (use argc - 1 because the first one is ./jobCommander which we didn't send over)
    char* argv[argc - 1];
    int i = 0;

    while (argument != NULL) {

        // Store a pointer to the argument in the array
        argv[i] = argument;

        // Get the next argument
        argument = strtok(NULL, " ");

        i++;
    }

    // Add NULL to the end of the array
    argv[argc - 1] = NULL;


    ////////////////////////////////////////////////////////////////////////
    // Check what the command is, and handle it appropriately

    // Command is issueJob
    if (strcmp(argv[0], "issueJob") == 0) {

        // If the number of running jobs is smaller than the Concurrency, the job can be executed isntantly
        if (running_queue->size < Concurrency) {
            
            // Call the function responsible for issuing the job to run
            Running_Job_Info* running_job_info = issue_running_job(NULL, argv, running_queue);

            // Format the exit message
            exit_message = format_r_job_info(running_job_info);


        }
        // Else issue the job to wait
        else {

            // Call the function responsible for issuing the job to wait
            Waiting_Job_Info* waiting_job_info = issue_waiting_job(argc, argv, waiting_queue);

            // Format the exit message
            exit_message = format_w_job_info(waiting_job_info);

        }
    }
    // Command is setConcurrency
    else if (strcmp(argv[0], "setConcurrency") == 0) {
        
        // Convert the parameter of the command from a string to an integer using atoi
        int new_concurrency = atoi(argv[1]);

        // Call the function responsible for setting the concurrency
        int result = set_concurrency(new_concurrency, waiting_queue, running_queue);

        switch (result) {

            case 0:

                // Format the exit message
                exit_message = "Concurrency set successfully\n";
                
                break;

            case -1:
            
                // Format the exit message
                exit_message = "Unable to set concurrency: Number given is invalid (Concurrency must be >=0)\n";

                break;
        }

    }
    // Command is stop
    else if (strcmp(argv[0], "stop") == 0) {

        // Store the jobID of the job that is to be stopped in a string
        char* jobID_to_search = argv[1];

        // Call the function responsible for stopping the job
        int result = stop_job(jobID_to_search, running_queue, waiting_queue);

        size_t message_size;

        // Format the exit message based on the function's result
        switch (result) {

            case 0:

                // Calculate the length of the job id + 12 for " terminated" and null terminator
                message_size = strlen(argv[1]) + 12;

                // Allocate memory for the exit message
                exit_message = malloc(message_size);

                // If allocation failed, exit with error code
                if (exit_message == NULL) {
                    perror("Failed to allocate memory");
                    exit(EXIT_FAILURE);
                }

                // Format the exit message
                snprintf(exit_message, message_size, "%s terminated\n", argv[1]);

                break;

            case 1:

                // Calculate the length of the job id + 9 for " removed" and null terminator
                message_size = strlen(argv[1]) + 9;

                // Allocate memory for the exit message
                exit_message = malloc(message_size);

                // If allocation failed, exit with error code
                if (exit_message == NULL) {
                    perror("Failed to allocate memory");
                    exit(EXIT_FAILURE);
                }

                // Format the exit message
                snprintf(exit_message, message_size, "%s removed\n", argv[1]);

                break;

            default:

                // Format the exit message
                exit_message = "Job is neither running nor waiting: Error\n";

                break;
        }
    }
    // Command is poll
    else if (strcmp(argv[0], "poll") == 0) {

        // If command is poll running, print the information about all processes in the running queue
        if (strcmp(argv[1], "running") == 0) {

            // Call the function responsible for formating the string that displays all the running jobs
            exit_message = poll_running(running_queue);
        }

        // If command is poll queued, print the information about all processes in the waiting queue
        else if (strcmp(argv[1], "queued") == 0) {

            // Call the function responsible for formating the string that displays all the waiting jobs
            exit_message = poll_waiting(waiting_queue);
        }
        // Else the command was invalid
        else {
            exit_message = "Invalid second argument for poll (must be either 'poll running' or 'poll queued')\n";
        }
    }
    // Command is "exit" (because we checked for invalid arguments in jobCommander)
    else {
        exit_message = "jobExecutorServer terminated.\n";
    }

    ////////////////////////////////////////////////////////////////////////
    // Write the message to jobCommander

    // Open the named pipe for writing to jobCommander
    file_desc_write = open(jes_to_jc_pipe, O_WRONLY);

    // If opening the pipe failed exit with error code
    if (file_desc_write == -1) {
        perror("Opening the pipe JES_TO_JC_PIPE failed");
        exit(EXIT_FAILURE);
    }

    // TESTING
    // printf("Named pipe opened for writing to jobCommander\n");

    // Write the exit message to the pipe
    if (write(file_desc_write, exit_message, strlen(exit_message)) == -1) {
        perror("Writing the exit_message to the named pipe failed");
        exit(EXIT_FAILURE);
    }

    // Close the named pipe for writing
    if (close(file_desc_write) == -1) {
        perror("Closing the named pipe failed");
        exit(EXIT_FAILURE);
    }

    unlink(jes_to_jc_pipe);

    ////////////////////////////////////////////////////////////////////////
    // Cleanup

    // If command was stop or exit, exit message was allocated dynamically so we free it
    if (strcmp(argv[0], "stop") == 0) {
        free(exit_message);
    }


    // If command was exit jobExecutorServer terminates
    if (strcmp(argv[0], "exit") == 0) {

        r_queue_destroy(running_queue);
        w_queue_destroy(waiting_queue);
        unlink(FILE_NAME);
        exit(EXIT_SUCCESS);

        
    }

}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


void sigchild_handler(int sig) {

    int child_status;

    // Load the first node of the queue
    Running_Queue_Node* node = running_queue->front;

    ////////////////////////////////////////////////////////////////////////
    // Find the job that terminated

    // Traverse the running queue to find the finished job
    while (node != NULL) {

        // Search for the child process that terminated but don't block the parent process
        if (waitpid(node->job_info->pid, &child_status, WNOHANG) != 0) {

            // If the process was found break out of the loop
            break;
        }
        // Else move on to the previous node
        else {
            node = node->previous;
        }
    }

    // If no job was found, return
    if (node == NULL) {
        perror("Failed to find the job that ended");
        return;
    }

    // Remove the job with the specific pid from the running queue
    Running_Job_Info* running_job_info = r_queue_remove_pid(node->job_info->pid, running_queue);

    // If removing the job failed, return
    if (running_job_info == NULL) {
        perror("Failed to remove the job that ended from the running queue");
        return;
    }

    // Free the memory allocated for the running job info
    // free(running_job_info);

    ////////////////////////////////////////////////////////////////////////
    // Check if there are jobs in the waiting queue to be issued

    Waiting_Job_Info* waiting_job_info;

    // If concurrency is larger than the size of the running queue, issue as many jobs as the concurrency and the size of the queue allow
    if (running_queue->size < Concurrency) {

        // The number of jobs to start executing is the difference between the concurrency and the current running queue size
        int jobs_to_be_issued_count = Concurrency - running_queue->size;

        for (int i = 0; i < jobs_to_be_issued_count; i++) {

            // If there are jobs in the waiting queue, issue them to start executing
            if (!is_w_queue_empty(waiting_queue)) {

                // Remove the job from the front of the waiting queue and store its info in waiting_job_info
                waiting_job_info = w_queue_remove(waiting_queue);

                // Issue the waiting job to run and add it to the running queue
                running_job_info = issue_running_job(waiting_job_info, NULL, running_queue);

                // Free the memory allocated for the waiting job info
                free(waiting_job_info);
                free(running_job_info);
            }
            // If there's no more jobs in the waiting queue, exit the loop
            else {
                break;
            }
        }
    }
}
