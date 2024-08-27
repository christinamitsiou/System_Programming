#include "header.h"

// Run with ./jobCommander issueJ
int main(int argc, char* argv[]) {
    
    // Initiallize the pipes
    char* jc_to_jes_pipe = JC_TO_JES_PIPE;
    char* jes_to_jc_pipe = JES_TO_JC_PIPE;

    // Initiallize the file descriptors for reading and writing
    int file_desc_write;
    int file_desc_read;

    int file_desc;
    int pid;

    char buff[BUFSIZ];

    ////////////////////////////////////////////////////////////////////////
    // Checking the input for errors or insufficient arguments

    // If only one argument was provided exit with error code
    if (argc == 1) {
        perror("Insufficient arguments");
        exit(EXIT_FAILURE);
    }
    // If the second argument is not one of the acceptable five options, exit with error code
    if ( (strcmp(argv[1],"issueJob") != 0) && (strcmp(argv[1],"setConcurrency") != 0) && (strcmp(argv[1],"stop") != 0) && (strcmp(argv[1],"poll") != 0) && (strcmp(argv[1],"exit") != 0)) {
        perror("Invalid argument(s)");
        exit(EXIT_FAILURE);
    }
    else {

        //////////////////////////////////////////////////////////////////////
        // Opening jobExecutorServer.txt, otherwise loading it's pid 

        // Attemp to open the file jobExecutorServer.txt
        file_desc = open(FILE_NAME, O_RDONLY);

        // Check if the file jobExecutorServer is open, and if it's not start executing it 
        if (file_desc < 0) {
            
            // Fork the process
            pid = fork();

            // If fork failed exit with error code
            if (pid < 0) {
                perror("Function fork() failed");
                exit(EXIT_FAILURE);
            }
            // If not, start jobExecutorServer automatically
            if (pid == 0) {
 
                execlp("./jobExecutorServer", "./jobExecutorServer", (char *)NULL);

                // If execlp returns exit with error code
                perror("execlp function failed");
                exit(EXIT_FAILURE);
            }

            // Give jobExevutorServer some time to start
            sleep(1);
            printf("jobExecutorServer opened\n");

            // Close the file
            close(file_desc);
        }
        // If jobExecutorServer's already open, read it's pid from the file
        else {
            
            printf("jobExecutorServer already open\n");

            // If reading from jobExecutorServer failed exit with error code
            if ( (read(file_desc, buff, sizeof(buff)) ) < 0) {  
                perror("Reading jobExecutorServer's pid failed");
                exit(EXIT_FAILURE);
            }

            // Convert jobExecutorServer's pid from a string to an integer
            pid = atoi(buff);
            
        }

        // Close the file
        // close(file_desc);

        ////////////////////////////////////////////////////////////////////////
        // Create the pipes

        // Check if the named pipe JES_TO_JB_PIPE exists, if not create it
        if (access(jes_to_jc_pipe, F_OK) == -1) {

            // If creating the pipe failed exit with error code
            if (mkfifo(jes_to_jc_pipe, 0666) == -1) {
                perror("mkfifo failed (JES_TO_JB_PIPE)");
                exit(EXIT_FAILURE);
            }

            // TESTING
            // printf("Named pipe JES_TO_JB_PIPE created successfully!\n");
        }
        
        // Check if the named pipe JC_TO_JES_PIPE exists, if not create it
        if (access(jc_to_jes_pipe, F_OK) == -1) {

            // If creating the pipe failed exit with error code
            if (mkfifo(jc_to_jes_pipe, 0666) == -1) {
                perror("mkfifo failed (JC_TO_JES_PIPE)");
                exit(EXIT_FAILURE);
            }

            // TESTING
            // printf("Named pipe JC_TO_JES_PIPE created successfully!\n");
        }

        // Send SIGUSR1 signal to jobExecutorServer
        kill(pid, SIGUSR1);


        ////////////////////////////////////////////////////////////////////////
        // Writing the arguments to jobExecutorServer

        // Open the named pipe for writing to jobExecutorServer
        file_desc_write = open(jc_to_jes_pipe, O_WRONLY);

        // If opening the pipe failed exit with error code
        if (file_desc_write == -1) {
            perror("Opening the pipe JC_TO_JES_PIPE failed");
            exit(EXIT_FAILURE);
        }
        
        // TESTING
        // printf("Named pipe opened for writing to jobExecutorServer\n");

        size_t arg_len;

        // Write the argument count to the named pipe
        if (write(file_desc_write, &argc, sizeof(argc)) == -1) {
            perror("Writing argc to the named pipe failed");
            exit(EXIT_FAILURE);
        }
        
        // Write the arguments to the named pipe one by one (except for the first one which is ./jobCommander)
        for (int i = 1; i < argc; i++) {
            
            // Calculate the argument length
            arg_len = strlen(argv[i]); 

            if (write(file_desc_write, argv[i], arg_len) == -1) {
                perror("Writing arguments to the named pipe failed");
                exit(EXIT_FAILURE);
            }

            // After all arguments but the last one, add a space between them to be able to tokenize them
            if(i != argc -1) {

                // If writing the space to the pipe failed exit with error code
                if (write(file_desc_write, " ", 1) == -1) {
                    perror("Writing space to the named pipe failed");
                    exit(EXIT_FAILURE);
                }
            }
        }

        // Write the null terminator after the last argument 
        if (write(file_desc_write, "\0", 1) == -1) {
            perror("Writing null terminator to the named pipe failed");
            exit(EXIT_FAILURE);
        }

        // Close the named pipe for writing to jobExecutorServer
        if (close(file_desc_write) == -1) {
            perror("Closing the named pipe failed");
            exit(EXIT_FAILURE);
        }

        // Unlink the pipe
        unlink(jc_to_jes_pipe);

        // Give jobExecutorServer time to format the message
        sleep(1);

        ////////////////////////////////////////////////////////////////////////
        // Reading the response from jobExecutorServer

        // Open the pipe for reading from jobExecutorServer
        file_desc_read = open(jes_to_jc_pipe, O_RDONLY);

        // If opening the pipe failed exit with error code
        if (file_desc_read == -1) {
            perror("Opening the pipe JES_TO_JB failed");
            exit(EXIT_FAILURE);
        }
        
        // TESTING
        // printf("Named pipe opened for reading from jobExecutorServer\n");

        ssize_t bytes_read;

        // Allocate buffer dynamically
        char* buffer = (char *)malloc(BUFSIZ);

        // If allocation failed exit with error code 
        if (buffer == NULL) {           
            perror("Allocating memory for buffer failed");
            exit(EXIT_FAILURE);
        }

        // Store its original size to use if needed for reallocation of the buffer
        int size_of_buffer = BUFSIZ; 

        // Read the arguments from jobExecutorServer
        bytes_read = read(file_desc_read, buffer, size_of_buffer);

        int shift_offset = 0;

        // Read the message until there's no more bytes to read
        while (bytes_read > 0) {

            // Shift the offset by the bytes read in the last while loop
            shift_offset += bytes_read;

            // If the offset is equal to/bigger than the size of the buffer, double the buffer
            if (shift_offset >= size_of_buffer) { 

                // Double the buffer size
                size_of_buffer *= 2;              

                // Reallocate memory for the buffer
                buffer = (char*)realloc(buffer, size_of_buffer);

                // If allocation failed exit with error code
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

        // If reading from the pipe failed exit with error code
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
        unlink(jes_to_jc_pipe);

        // Print the message received from jobExecutorServer
        printf("%s", buffer); 

        // Free the memory allocated for the buffer
        free(buffer);  

    }

    exit(EXIT_SUCCESS);
}
