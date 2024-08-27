#include "Thread_Functions.h"

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void* main_thread_func(void* arg) {

    // TESTING
    // printf("Main thread is running.\n");

    int newsock,result;
    socklen_t clientlen;

    // In the beginning the buffer is not full
    shared_data->buf_not_full = true;

    // In the beginning the buffer is empty
    shared_data->buf_not_empty = false;

    // A flag to stop the loop when the input exit is given
    shared_data->exiting = false;

    // Get the numbr of worker threads aka the ThreadPoolSize from the arguments
    int worker_threads_num = *( int *) arg;

    // Initialize an array of worker threads of size working_threads_num
    pthread_t worker_threads[worker_threads_num];

    // Initialize worker threads
    for (int i = 0; i < worker_threads_num; i++) {

        // If thread creation failed exit with error code
        int result = pthread_create(&worker_threads[i], NULL, worker_thread_func, NULL);

        // If creating the threads failed exit with error code
        if (result!= 0) {
            fprintf(stderr, "Error creating working thread:%s\n",strerror(result));
            exit(EXIT_FAILURE);
        }
    }

    // Initialize a dynamic array of pointers to controller threads 
    pthread_t* controller_threads = (pthread_t*)malloc(sizeof(pthread_t));

    // If allocation failed exit with error code
    if (controller_threads == NULL) {
        fprintf(stderr, "Failed to allocate initial memory for threads\n");
        exit(EXIT_FAILURE);
    }

    // Initialize controller threads number to 0
    int controller_threads_num = 0;

    // While the server is not exiting
    while (!(shared_data->exiting)) {

        clientlen = sizeof(shared_data->client);

        // Accept connection and if it fails continue
        if ((newsock = accept(shared_data->sock, shared_data->clientptr, &clientlen)) < 0) {
            continue;
        }

        // TESTING
        printf("Accepted connection\n");

        // Create a controller thread
        result = pthread_create(&(controller_threads[controller_threads_num]), NULL, controller_thread_func, (void*)&newsock);
        
        // If thread creation failed exit with error code
        if (result != 0) {

            fprintf(stderr, "Error creating thread: %s\n", strerror(result));

            // Close the sock
            close(newsock);

            // Free allocated memory  before exiting
            free(controller_threads); 
            exit(EXIT_FAILURE);
        }


        // Increase the number of controller threads
        controller_threads_num++;

        // Reallocate memory to be able to store another thread
        controller_threads = (pthread_t*)realloc(controller_threads, (controller_threads_num + 1) * sizeof(pthread_t));

        // If realloc failed exit with error code
        if (controller_threads == NULL) {
            
            fprintf(stderr, "Failed to reallocate memory for threads\n");

            exit(EXIT_FAILURE);
        }


    }

    // Print an exiting message
    printf("SERVER IS SHUTING DOWN...\n");

    // Wait for all the worker threads to finish 
    for (int i = 0; i < worker_threads_num; i++) {

        result = pthread_join(worker_threads[i], NULL);

        if (result != 0) {
            fprintf(stderr,"join%s\n",strerror(result));
        }

    }

    // Wait for all the controller threads to finish 
    for (int i = 0; i < controller_threads_num; i++) {

        result = pthread_join(controller_threads[i], NULL);
        
        if (result != 0) {
            fprintf(stderr, "Error joining thread %d: %s\n", i, strerror(result));
        }
    }

    free(controller_threads);

    // Close the initial socket
    close(shared_data->sock);

    pthread_exit(NULL);

}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


void* worker_thread_func(void* arg) {

    // TESTING
    // printf("Worker thread is running.\n");

    // While the server is not exiting
    while (!(shared_data->exiting)) {

        //////////////////////////////////////////////////////////
        // The worker thread sleeps until there's a job in the buffer

        // Lock the not_empty mutex
        pthread_mutex_lock(&(shared_data->buf_not_empty_mutex));

        // Wait until the buffer is not empty
        while (!(shared_data->buf_not_empty)) {

            pthread_cond_wait(&(shared_data->buf_not_empty_cond), &(shared_data->buf_not_empty_mutex));

            // Lock the exiting mutex
            pthread_mutex_lock(&(shared_data->exiting_mutex));

            // If while waiting the exit signal is received exit
            if (shared_data->exiting) {

                // Unlock the exiting mutex
                pthread_mutex_unlock(&(shared_data->exiting_mutex));

                // Unlock the not_empty mutex
                pthread_mutex_unlock(&(shared_data->buf_not_empty_mutex));
                pthread_exit(NULL);

            }

            // Unlock the exiting mutex
            pthread_mutex_unlock(&(shared_data->exiting_mutex));
            }

        // Unlock the not_empty mutex
        pthread_mutex_unlock(&(shared_data->buf_not_empty_mutex));
        
        //////////////////////////////////////////////////////////
        // Execute jobs from the buffer

        // Lock mutex to access queue
        pthread_mutex_lock(&(shared_data->buffer_mutex));

        // Lock mutexes to access members of shared data
        pthread_mutex_lock(&(shared_data->jobs_running_mutex));
        pthread_mutex_lock(&(shared_data->concurrency_mutex));

        // If there's a job in the queue and the concurrencyLevel allows for it to be run, execute it
        if (jobs_currently_running < concurrencyLevel) {

            // Remove the job from the front of the queue
            Job_Info* job_to_execute = queue_remove(buffer);
            
            //////////////////////////////////////////////////////////
            // Signal that the buffer is not full

            // Lock the buf_not_full mutex
            pthread_mutex_lock(&(shared_data->buf_not_full_mutex));

            // If removing the job was successfull make the buf_not_full variable true
            if (job_to_execute != NULL)  {

                shared_data->buf_not_full = true;

                // Signal that the buffer is not full to wake up any controller threads waiting to insert jobs in the buffer
                pthread_cond_signal(&(shared_data->buf_not_full_cond));
            }

            pthread_mutex_unlock(&(shared_data->buf_not_full_mutex));

            //////////////////////////////////////////////////////////
            // Check if the removal of the job left the buffer empty

            // Lock the buf_not_empty mutex
            pthread_mutex_lock(&(shared_data->buf_not_empty_mutex));

            // If the queue is now empty update the variable
            if (is_queue_empty(buffer)) {

                shared_data->buf_not_empty = false;

            }
            
            // Unlock the buf_not_empty mutex
            pthread_mutex_unlock(&(shared_data->buf_not_empty_mutex));


            // Create child for serving client 
            pid_t pid = fork();

            // If fork() failed exit with error code
            if (pid < 0) {

                fprintf(stderr,"fork() failed");

                // Free allocated memory for job info
                destroy_job_info(job_to_execute);
                
                exit(EXIT_FAILURE);
            }
            // Child process
            else if (pid == 0) { 
                child_server(job_to_execute);
            }
            // Parent process
            else {

                // Increase the jobs currently running by one
                jobs_currently_running += 1;

                int status;

                // Wait for the child process to finish executing
                waitpid(pid, &status, 0);

                // Decrease the jobs currently running by one
                jobs_currently_running -= 1;

                pthread_mutex_unlock(&(shared_data->jobs_running_mutex));
                pthread_mutex_unlock(&(shared_data->concurrency_mutex));

                // Unlock buffer mutex 
                pthread_mutex_unlock(&(shared_data->buffer_mutex));

                char output_filename[50];

                // Get the output file's name based on the pid of the child
                snprintf(output_filename, sizeof(output_filename), "%d.output", pid);

                // Open the file for reading
                int output_file_desc = open(output_filename, O_RDONLY);

                // If opening the file failed exit with error code
                if (output_file_desc == -1) {
                    fprintf(stderr,"Opening the output file failed");
                    exit(EXIT_FAILURE);
                }

                // Use helping function to format the file output
                char* file_output = format_file_output(output_file_desc,job_to_execute->jobID);

                // If writing the formated file output to the socket failed exit with error code
                if (write(job_to_execute->clientSocket, file_output, strlen(file_output)) < 0) {
                    fprintf(stderr,"Writing output to the socket failed\n");
                    exit(EXIT_FAILURE);
                }

                // Shut down the write flow of the sock 
                if (shutdown(job_to_execute->clientSocket,SHUT_WR)== -1) {
                    fprintf(stderr,"shutdown failed\n");
                    exit(EXIT_FAILURE);
                }

                // Free the formated file output string
                free(file_output);

                // Close the file and delete it
                close(output_file_desc);
                unlink(output_filename);

                // Everything's done so close the client socket
                close(job_to_execute->clientSocket);

                // Free allocated memory for job info
                destroy_job_info(job_to_execute);

            }

        }

        else  {
            pthread_mutex_unlock(&(shared_data->concurrency_mutex));
            pthread_mutex_unlock(&(shared_data->jobs_running_mutex));
            pthread_mutex_unlock(&(shared_data->buffer_mutex));
        }
        
    }

    pthread_exit(NULL);

}



// /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


void* controller_thread_func(void* arg) {
    
    // TESTING
    // printf("Controller thread is running.\n");

    // Get the new sock from the argument
    int newsock = *( int *) arg;

    int result;

    ////////////////////////////////////////////////////////////////
    // Read the input from client

    ssize_t bytes_read;

    // Allocate buffer dynamically
    char* input_buf = (char*) malloc(BUFSIZ);

    // If allocation failed exit with error code 
    if (input_buf == NULL) {           
        fprintf(stderr,"Allocating memory for buffer failed");
        exit(EXIT_FAILURE);
    }

    // Store it's original size to use if needed for reallocation of the buffer
    int size_of_input_buf = BUFSIZ; 

    int argc;

    // Read the argument count from the sock
    bytes_read = read(newsock, &argc, sizeof(argc));

    // If reading from the socket failed, exit with error code
    if (bytes_read == -1) {
        fprintf(stderr,"read");
        exit(EXIT_FAILURE);
    }

    // TESTING
    // printf("Received %d arguments\n", argc);

    // Read the rest of the arguments from jobCommander client
    bytes_read = read(newsock, input_buf, size_of_input_buf);

    // If reading from the socket failed, exit with error code
    if (bytes_read == -1) {
        fprintf(stderr,"read");
        exit(EXIT_FAILURE);
    }

    int shift_offset = 0;

    // Read the message until there's no more bytes to read
    while (bytes_read > 0) {
        
        // Shift the offset by the bytes read in the last while loop
        shift_offset += bytes_read;

        // If the offset is equal to/bigger than the size of the buffer, double the buffer
        if (shift_offset >= size_of_input_buf) { 

            // Double the buffer size
            size_of_input_buf *= 2;              

            // Reallocate memory for the buffer
            input_buf = (char*)realloc(input_buf, size_of_input_buf);

            // If allocation failed exit with error code
            if (input_buf == NULL) {  
                fprintf(stderr,"Reallocating memory for buffer failed");
                exit(EXIT_FAILURE);
            }
        }

        // The next set of bytes to be read is stored in the buffer after the last round of bytes (using shift to get to that position)
        bytes_read = read(newsock, input_buf + shift_offset, size_of_input_buf - shift_offset);

    }

    // If reading from the socket failed, exit with error code
    if (bytes_read == -1) {
        fprintf(stderr,"Reading from the socket failed");
        free(input_buf);
        exit(EXIT_FAILURE);
    }

    // Make the input null-terminated
    input_buf[shift_offset] = '\0';

    // TESTING
    // printf("Message from client is: %s\n", input_buf);

    // Shutdown the read flow of the sock as we don't need to read from the client any more
    if (shutdown(newsock,SHUT_RD)== -1) {
        fprintf(stderr,"shutdown");
        exit(EXIT_FAILURE);
    }

    //////////////////////////////////////////////////////////////////////////////////////
    // Tokenize the input so we can better process it

    // Start reading the arguments from buffer
    char* argument = strtok(input_buf, " ");

    // Create an array of pointers to each argument + 1 for the NULL terminator
    char* argv[argc + 1];  
    int i = 0;

    while (argument != NULL) {

        // Store a pointer to the argument in the array
        argv[i] = argument;

        // TESTING
        // printf("argv[%d] is %s\n",i,argv[i]);

        // Get the next argument
        argument = strtok(NULL, " ");

        i++;
    }

    // Add NULL terminator to the end of the array
    argv[argc] = '\0';

    //////////////////////////////////////////////////////////////////////////////////////
    // Check what the command is, and handle it appropriately

    char* exit_message;

    // Command is issueJob
    if (strcmp(argv[0], "issueJob") == 0) {

        // If there's less than 2 arguments exit with error code
        if (argc < 2) {
            perror("Need at least 1 more argument for the issueJob command to be executed\n");
        }

        //////////////////////////////////////////////////////////////////////////////////////
        // Store the information for the new job

        // Allocate memory for the new job's info
        Job_Info* new_job_info = malloc(sizeof(Job_Info));    

        // Create a unique ID for the job that is to be issued
        new_job_info->jobID = convert_jobID_to_string();

        // If creating the id failed exit with error code
        if (new_job_info->jobID == NULL) {

            fprintf(stderr,"Job ID creation failed\n");

            // Free previously the allocated memory for job_info
            free(new_job_info); 
            exit(EXIT_FAILURE);
        }

        // Store the name of the job
        new_job_info->job = strdup(argv[1]);

        // If strdup failed exit with error code
        if (new_job_info->job == NULL) {
            
            fprintf(stderr,"Job name duplication failed\n");

            // Free previously allocated memory for job_info
            free(new_job_info->jobID);
            free(new_job_info); 
            exit(EXIT_FAILURE);
        }    
        
        // Store the client socket
        new_job_info->clientSocket = newsock;

        // Allocate memory for the parameters array
        new_job_info->parameters = malloc((argc) * sizeof(char *));

        // If allocation failed exit with error code
        if (new_job_info->parameters == NULL) {
            fprintf(stderr, "Memory allocation for parameters failed\n");
            free(new_job_info->jobID);
            free(new_job_info->job);
            free(new_job_info);
            exit(EXIT_FAILURE);
        }

        // Store the parameters in the array
        for (int i = 0; i < argc - 1; i++) {

            new_job_info->parameters[i] = strdup(argv[i + 1]);

            // If strdup failed exit with error code
            if (new_job_info->parameters[i] == NULL) {

                fprintf(stderr, "Storing the parameters failed\n");

                // Free previously allocated memory
                free(new_job_info->jobID);
                free(new_job_info->job);

                for (int j = 0; j < i; j++) {
                    free(new_job_info->parameters[j]);
                }
                free(new_job_info->parameters);
                free(new_job_info);
                exit(EXIT_FAILURE);
            }

            // TESTING
            // printf("Parameters: %s\n", new_job_info->parameters[i]);
        }

        // Store the number of parameters
        new_job_info->num_of_parameters = argc - 1;

        // NULL terminate the parameters array
        new_job_info->parameters[argc-1] = NULL;

        //////////////////////////////////////////////////////////////////////////////////////
        // Wait for the buffer to have space before inserting the job

        // Lock buf_not_full mutex
        pthread_mutex_lock(&(shared_data->buf_not_full_mutex));

        // If the buffer is full wait until the buf_not_full_cond has been signaled
        while (!(shared_data->buf_not_full)) {

            pthread_cond_wait(&(shared_data->buf_not_full_cond),&(shared_data->buf_not_full_mutex));

            // Lock the exiting mutex to access exiting variable
            pthread_mutex_lock(&(shared_data->exiting_mutex));

            // If at any point when waiting the server exits return
            if (shared_data->exiting) {

                // Unlock the exiting mutex
                pthread_mutex_unlock(&(shared_data->exiting_mutex));

                // Unlock the buf_not_full mutex
                pthread_mutex_unlock(&(shared_data->buf_not_full_mutex));

                // Sent an appropriate message to the client that issued the job
                char* exit_message = "SERVER TERMINATED BEFORE EXECUTION";

                // If writing the exit message failed exit with error code
                if (write(newsock, exit_message, strlen(exit_message)) < 0) {
                    fprintf(stderr,"Writing message to the client failed here\n");
                    exit(EXIT_FAILURE);
                }

                // Close the client socket
                close(newsock);
                pthread_exit(NULL);
            }

            // Unlock the exiting mutex
            pthread_mutex_unlock(&(shared_data->exiting_mutex));
        
            
        }
        
        // Unlock the buf_not_full mutex
        pthread_mutex_unlock(&(shared_data->buf_not_full_mutex));

        //////////////////////////////////////////////////////////////////////////////////////
        // Insert the job to the buffer (queue)

        // Lock buffer mutex to access the buffer
        pthread_mutex_lock(&(shared_data->buffer_mutex));
        
        // Submit the job to the buffer
        result = queue_insert(buffer,new_job_info);

        // TESTING
        // print_queue(buffer);

        // If inserting the job to the buffer failed exit with error code
        if (result != 0) {

            perror("Failed to submit job\n");

            destroy_job_info(new_job_info);
            exit(EXIT_FAILURE);
        }

        //////////////////////////////////////////////////////////////////////////////////////
        // Update the global variables if the buffer is now full

        // Lock the not_full mutex
        pthread_mutex_lock(&(shared_data->buf_not_full_mutex));

        // If the insertion of the new job made the buffer full change the global variable
        if (is_queue_full(buffer)) {
            shared_data->buf_not_full = false;
        }

        // Unlock the not_full mutex
        pthread_mutex_unlock(&(shared_data->buf_not_full_mutex));

        // Unlock the buffer mutex
        pthread_mutex_unlock(&(shared_data->buffer_mutex));

        //////////////////////////////////////////////////////////////////////////////////////
        // Wake up working threads by signaling that the buffer is now not empty

        // Lock the not_empty mutex
        pthread_mutex_lock(&(shared_data->buf_not_empty_mutex));

        // If buffer was previously empty update the buffer
        if (!(shared_data->buf_not_empty)) {

            // Make the not_empty variable true
            shared_data->buf_not_empty = true;

            // Signal that the buffer is not empty
            pthread_cond_signal(&(shared_data->buf_not_empty_cond));

 
        }

        // Unlock the not_empty mutex
        pthread_mutex_unlock(&(shared_data->buf_not_empty_mutex));

        // Format the exit message
        exit_message = format_issued_job_info(new_job_info);

    }
    // Command is setConcurrency
    else if (strcmp(argv[0], "setConcurrency") == 0) {

        if (argc == 1) {
            fprintf(stderr,"Failed to set concurrency: No number was provided\n");
            exit(EXIT_FAILURE);
        }
        else {

            // Get the new concurrency from the arguments
            int new_concurrency = atoi(argv[1]);

            result = set_concurrency(new_concurrency);

            if (result < 0) {

                fprintf(stderr,"Failed to set concurrency: Must be a positive number\n");

                exit(EXIT_FAILURE);
            }

            exit_message = format_concurrency_set(concurrencyLevel);
        }

    }
    // Command is stop
    else if (strcmp(argv[0], "stop") == 0) {

        // Get the jobID of the job that will be stopped from the arguments
        char* jobID_to_stop = argv[1];

        // Remove the job from the buffer-queue
        int result = stop_job(jobID_to_stop,buffer);

        // If result is 0 the removal wa successfull
        if (result == 0) {
            exit_message = format_removed_jobID(jobID_to_stop);


        }
        else {
            exit_message = format_not_found_jobID(jobID_to_stop);
        }

    }
    // Command is poll
    else if (strcmp(argv[0], "poll") == 0) {

        exit_message = format_poll_output(buffer);

    }
    else {

        Queue_Node* node = buffer->front;
        Job_Info* info;
        int client_sock;

        exit_message = "SERVER TERMINATED BEFORE EXECUTION";

        // Go through all the nodes of the queue
        while (node != NULL) {

            client_sock = node->job_info->clientSocket;

            // For every client that had a job in the buffer send the terminating message
            if (write(client_sock, exit_message, strlen(exit_message)) < 0) {
                fprintf(stderr,"Writing message to the client failed here\n");
                exit(EXIT_FAILURE);
            }

            // Close the client socket
           close(client_sock);

            // Go to the next node
            node = node->previous;

            // Remove the job from the front of the buffer-queue
            info = queue_remove(buffer);
            if (info == NULL) {
                fprintf(stderr,"Error removing the front of the queue\n");

            }
            else {
                destroy_job_info(info);
            }
        }

        exit_message = "SERVER TERMINATED";

        free(input_buf); 

        // Write the exit message to the client that ordered the exit command
        if (write(newsock, exit_message, strlen(exit_message)) < 0) {
            fprintf(stderr,"Writing response to the client failed");
            exit(EXIT_FAILURE);
        }
        // TESTING
        // printf("Closing connection\n");

        pthread_mutex_lock(&(shared_data->exiting_mutex));

        // Set exiting to true
        shared_data->exiting = true;

        pthread_mutex_unlock(&(shared_data->exiting_mutex));

        
        // Closing connection to the client that ordered the exit command
        close(newsock);


        // Shut down the read flow of the initial sock so the accept call get's unblocked
        if (shutdown(shared_data->sock,SHUT_RD)== -1) {
            fprintf(stderr,"shutdown");
            exit(EXIT_FAILURE);
        }

        ////////////////////////////////////////////////////////////////////////////////////
        // Wake up all working threads 

        pthread_mutex_lock(&(shared_data->buf_not_empty_mutex));

        shared_data->buf_not_empty = true;

        // Signal all worker threads to wake up
        pthread_cond_broadcast(&(shared_data->buf_not_empty_cond));

        pthread_mutex_unlock(&(shared_data->buf_not_empty_mutex));

        //////////////////////////////////////////////////////////////////////////////////////
        // Wake up all controller threads

        pthread_mutex_lock(&(shared_data->buf_not_full_mutex));

        shared_data->buf_not_full = true;

        // Signal all controller threads to wake up
        pthread_cond_broadcast(&(shared_data->buf_not_full_cond));

        pthread_mutex_unlock(&(shared_data->buf_not_full_mutex));

        pthread_exit(NULL);

    }

    // If writing the response to the client failed exit with error code
    if (write(newsock, exit_message, strlen(exit_message)) < 0) {
        fprintf(stderr,"Writing response to the client failed");
        exit(EXIT_FAILURE);
    }

    
    // Closing connections for all commands except issueJob
    if (strcmp(argv[0], "issueJob") != 0) {
        // TESTING
        // printf("Closing connection\n");
        close(newsock);

    }
    // If exit message was allocated dynamically free it
    if ((exit_message!=NULL) && (strcmp(exit_message, "No jobs in the buffer at the moment\n") != 0)) {
        free(exit_message);
    }
    // Free the input buffer
    if (input_buf!=NULL) {
        free(input_buf);
    }

    pthread_exit(NULL);

}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void child_server(Job_Info* job_to_execute) {

    // Create the output file with the format "pid.output"
    char output_filename[50];

    snprintf(output_filename, sizeof(output_filename), "%d.output", getpid());

    // Open the file for writing
    int output_fd = open(output_filename, O_WRONLY | O_CREAT | O_TRUNC, 0644);

    // If opening the file failed exit with error code
    if (output_fd < 0) {
        fprintf(stderr,"open");
        exit(EXIT_FAILURE);
    }

    // Redirect standart output to the output file
    if (dup2(output_fd, STDOUT_FILENO) < 0) {
        fprintf(stderr,"dup2");
        close(output_fd);
        exit(EXIT_FAILURE);
    }

    // Close the original file descriptor
    close(output_fd);

    // Execute the job
    execvp(job_to_execute->job, job_to_execute->parameters);

    // If execvp returns, an error occurred
    fprintf(stderr,"execvp failed\n");

    exit(EXIT_FAILURE);

}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////



