#include "Queue.h"

// ./bin/./jobCommander localhost 9002 issueJob ls -l
// ./bin/./jobCommander localhost 9002 issueJob bin/./progDelay 10
// ./bin/./jobCommander localhost 9002 issueJob bin/./hello_world
// ./bin/./jobCommander localhost 9002 issueJob bin/./capital_lowercase g
// ./bin/./jobCommander localhost 9002 exit
// ./bin/./jobCommander localhost 9002 poll
// ./bin/./jobCommander localhost 9002 stop Job_2
// ./bin/./jobCommander localhost 9002 setConcurrency 2

int main(int argc, char* argv[]) {

    ////////////////////////////////////////////////////////////////
    // Check for invalid input

    // If not enough arguments were provided exit with error 
    if (argc < 4) {
        perror("ERROR: Insufficient arguments\n");
        exit(EXIT_FAILURE);
    }

    // If the third argument is not an acceptable number for a port exit with error code
    int portNum = atoi(argv[2]);
    if (portNum < 1) {
        perror("ERROR: Invalid number for portNum\n");
        exit(EXIT_FAILURE);
    }

    // If the fourth argument is not one of the acceptable five options, exit with error code
    if ((strcmp(argv[3], "issueJob") != 0) && (strcmp(argv[3], "setConcurrency") != 0) && (strcmp(argv[3], "stop") != 0) && (strcmp(argv[3], "poll") != 0) && (strcmp(argv[3], "exit") != 0)) {
        perror("ERROR: Invalid command\n");
        exit(EXIT_FAILURE);
    }

    ////////////////////////////////////////////////////////////////
    // Resolve hostname

    char symbolicip[50];
    struct in_addr** addr_list;

    // Resolve the serverName
    struct hostent* mymachine = gethostbyname(argv[1]); 

    // If resolving the serverName failed exit with error code
    if (mymachine == NULL) {
        perror("Could not resolve Name\n");
        exit(EXIT_FAILURE);
    }
    else {
        // Store the list of IP addresses in addr_list
        addr_list = (struct in_addr**) mymachine->h_addr_list;

        // Go through every IP address in the list
        for (int i = 0; addr_list[i] != NULL; i++) {

            // Store the converted IP address in the symbolicip array
            strcpy(symbolicip, inet_ntoa(*addr_list[i]));

            // TESTING
            // printf("%s resolved to %s\n", mymachine->h_name, symbolicip);
        }
    }

    ////////////////////////////////////////////////////////////////
    // Connect to jobExecutorServer

    int sock;
    struct sockaddr_in server;
    struct sockaddr *serverptr = (struct sockaddr *)&server;
    struct hostent *rem;

    // Create socket 
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Creating the socket failed");
    }

    // Find server address 
    if ((rem = gethostbyname(argv[1])) == NULL) {
        herror("gethostbyname() failed");
        exit(1);
    }

    // Internet domain 
    server.sin_family = AF_INET; 
    memcpy(&server.sin_addr, rem->h_addr_list[0], rem->h_length);

    // The port provided 
    server.sin_port = htons(portNum);

    // Connect to the socket
    if (connect(sock, serverptr, sizeof(server)) < 0)
        perror("Connect to the socket failed");

    // printf("Connecting to %s port %d\n", argv[1], portNum);

    ////////////////////////////////////////////////////////////////
    // Write the arguments to jobExecutorServer

    // The first 3 arguments are not sent over
    int argc_2 = argc - 3;

    // Write the argument count first
    if (write(sock, &argc_2 , sizeof(argc_2)) < 0) {
        perror("Writing argument count to the socket failed");
        exit(EXIT_FAILURE);
    }

    size_t arg_len;

    // Write the arguments to the socket one by one (except for the first 3 which are ./jobCommmander [serverName] [portNum])
    for (int i = 3; i < argc; i++) {

        // Calculate the argument length
        arg_len = strlen(argv[i]); 

        // If writing the argument failed exit with error code
        if (write(sock, argv[i], arg_len) < 0) {
            perror("Writing arguments to the socket failed");
            exit(EXIT_FAILURE);
        }

        // Make sure to add a space between them to be able to tokenize them (but not on the last argument)
        if (i != argc - 1) {
            // If writing the space failed exit with error code
            if (write(sock, " ", 1) == -1) {
                perror("Writing space to the socket failed");
                exit(EXIT_FAILURE);
            }
        }
    }

    // Write the null terminator after the last argument
    if (write(sock, "\0", 1) == -1) {
        perror("Writing null terminator to the socket failed");
        exit(EXIT_FAILURE);
    }

    // Shut down the write flow of the sock
    if (shutdown(sock,SHUT_WR)== -1) {
        perror("Shutdown of the write flow failed");
        exit(EXIT_FAILURE);
    }


    // TESTING
    // printf("Writing arguments was successfull\n");

    ////////////////////////////////////////////////////////////////
    // Read the response from jobExecutorServer

    ssize_t bytes_read;

    // Allocate buffer dynamically
    char* response_buf = (char *)malloc(BUFSIZ);

    // If allocation failed exit with error code 
    if (response_buf == NULL) {           
        perror("Allocating memory for buffer failed");
        exit(EXIT_FAILURE);
    }

    // Store its original size to use if needed for reallocation of the buffer
    int size_of_buffer = BUFSIZ; 

    // Read the arguments from jobExecutorServer
    bytes_read = read(sock, response_buf, size_of_buffer);

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
        if (shift_offset >= size_of_buffer) { 

            // Double the buffer size
            size_of_buffer *= 2;              

            // Reallocate memory for the buffer
            response_buf = (char*)realloc(response_buf, size_of_buffer);

            // If allocation failed exit with error code
            if (response_buf == NULL) {   
                perror("Reallocating memory for buffer failed");
                exit(EXIT_FAILURE);
            }
        }
        
        // The next set of bytes to be read is stored in the buffer after the last round of bytes (using shift to get to that position)
        bytes_read = read(sock, response_buf + shift_offset, size_of_buffer - shift_offset);
    }

    // If reading from the socket failed exit with error code
    if (bytes_read == -1) { 
        perror("Reading from the server failed");
        exit(EXIT_FAILURE);
    }

    // Ensure null-terminated response
    response_buf[shift_offset] = '\0';

    printf("\n%s\n",response_buf);

    // Close socket 
    close(sock); 

    // Free the buffer
    free(response_buf);
    
    return 0;
}
