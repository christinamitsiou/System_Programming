#include "Thread_Functions.h"

// ./bin/./jobExecutorServer 9002 8 3
// ./bin/./jobExecutorServer 9002 1 10
// ./bin/./jobExecutorServer 9002 10 1

// Initialize the global variables
int jobID_as_num = 1;
int concurrencyLevel = 1;
int jobs_currently_running = 0;

int main(int argc, char* argv[]) {

    ////////////////////////////////////////////////////////////////
    // Check for invalid input

    // If not enough arguments were provided exit with error 
    if (argc != 4) {
        perror("Please give all following arguments: [portnum], [bufferSize], [threadPoolSize]\n");
        exit(EXIT_FAILURE);
    }

    int portNum = atoi(argv[1]);

    // If portNum is not an acceptable number exit with error code
    if (portNum < 1) {
        perror("ERROR: Invalid number for portNum\n");
        exit(EXIT_FAILURE);
    }

    int bufferSize = atoi(argv[2]);

    // If bufferSize is not an acceptable number exit with error code
    if (bufferSize <= 0) {
        perror("ERROR: Invalid number for portNum\n");
        exit(EXIT_FAILURE);
    }

    int threadPoolSize = atoi(argv[3]);

    // If threadPoolSize is not an acceptable number exit with error code
    if (threadPoolSize <= 0) {
        printf("ERROR: Invalid number for threadPoolSize\n");
        exit(EXIT_FAILURE);
    }

    //////////////////////////////////////////////////////////////
    // Initialize bufffer, main thread etc

    // If bufferSize is acceptable initiallize the global buffer
    buffer = queue_create(bufferSize);

    int sock;
    struct sockaddr_in server;
    struct sockaddr_in client;

    struct sockaddr* serverptr = (struct sockaddr *)&server;
    struct sockaddr* clientptr = (struct sockaddr *)&client;

    // Create a socket 
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Creating the socket failed");
    }

    int opt = 1;

    if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt))) {
        perror("setsockopt() failed");
        close(sock);
        exit(EXIT_FAILURE);
    }

    // Internet domain 
    server.sin_family = AF_INET; 
    server.sin_addr.s_addr = htonl(INADDR_ANY);

    // The port provided 
    server.sin_port = htons(portNum); 

    // Bind socket to address 
    if (bind(sock, serverptr, sizeof(server)) < 0)
        perror("Binding socket to address failed");

    // Listen for connections 
    if (listen(sock, CONNECTIONS) < 0)
        perror("Listening for connections failed");

    // TESTING
    printf("Listening for connections to port %d\n", portNum);

    // Allocate memory for the global struct shared data
    shared_data = malloc(sizeof(Shared_Data));

    // If allocation failed eit with error code
    if (shared_data == NULL) {
        perror("Memory allocation for shared data failed\n");
        exit(EXIT_FAILURE);
    }

    // Store the information in the global struct shared_data
    shared_data->sock = sock;
    shared_data->client = client;
    shared_data->clientptr = clientptr;

    // Initialize mutexes 
    pthread_mutex_init(&(shared_data->exiting_mutex), NULL);
    pthread_mutex_init(&(shared_data->buffer_mutex), NULL);
    pthread_mutex_init(&(shared_data->jobs_running_mutex), NULL);
    pthread_mutex_init(&(shared_data->concurrency_mutex), NULL);
    pthread_mutex_init(&(shared_data->jobID_mutex), NULL);
    pthread_mutex_init(&(shared_data->buf_not_empty_mutex), NULL);
    pthread_mutex_init(&(shared_data->buf_not_full_mutex), NULL);

    // Initialize condition variables
    pthread_cond_init(&(shared_data->buf_not_empty_cond), NULL);
    pthread_cond_init(&(shared_data->buf_not_full_cond), NULL);

    // Initialize main thread
    pthread_t main_thread;
    int result;

    // Create the thread with threadPoolSize as input
    result = pthread_create(&main_thread, NULL, main_thread_func, (void*)&threadPoolSize);
    
    // If creating the thread failed exit with error code
    if (result != 0) {
        fprintf(stderr, "Error creating thread\n");
        exit(EXIT_FAILURE);
    }

    // Wait for the thread to finish
    result = pthread_join(main_thread, NULL);

    // If joining the thread failed exit with error code
    if (result != 0) {
        fprintf(stderr, "Error joining thread\n");
        exit(EXIT_FAILURE);
    }

    // TESTING
    // printf("Main thread has finished executing.\n");

    //////////////////////////////////////////////////////////////
    // Clean up

    // Destroy the mutexes
    pthread_mutex_destroy(&(shared_data->exiting_mutex));
    pthread_mutex_destroy(&(shared_data->buffer_mutex));
    pthread_mutex_destroy(&(shared_data->jobs_running_mutex));
    pthread_mutex_destroy(&(shared_data->concurrency_mutex));
    pthread_mutex_destroy(&(shared_data->jobID_mutex));
    pthread_mutex_destroy(&(shared_data->buf_not_empty_mutex));
    pthread_mutex_destroy(&(shared_data->buf_not_full_mutex));

    
    pthread_cond_destroy(&(shared_data->buf_not_empty_cond));
    pthread_cond_destroy(&(shared_data->buf_not_full_cond));

    if (shared_data!=NULL) {
        free(shared_data);
    }

    queue_destroy(buffer);
    
    return 0;

}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

