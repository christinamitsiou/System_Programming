#include "JES_helping_functions.h"

// Run with ./JobExecutorServer

// jobID_as_num starts off at 0
int jobID_as_num = 0;

int main() {

    int file_desc;
    char buff[BUFSIZ];
    int pid;

    // Initiallize concurrency to 1
    Concurrency = 1;

    // Initiallize an empty waiting queue
    waiting_queue = w_queue_create();

    // Initialize an empty running queue
    running_queue = r_queue_create();

    // If the file exists, delete it
    unlink("jobExecutorServer.txt");

    // Set file permissions to read/write for user and read for others
    mode_t permissions = S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH;

    // Create and open the jobExecutorServer.txt file
    if ((file_desc = open(FILE_NAME, O_CREAT | O_TRUNC | O_WRONLY, permissions)) < 0) {
        perror("Creating jobExecutorServer.txt failed");
        exit(EXIT_FAILURE);
    }

    // Get jobExecutorServer's pid
    pid = getpid();

    // Store it as a string in buffer
    snprintf(buff, sizeof(buff), "%d", pid);

    // Write the pid to the file and if it failed, exit with error code
    if (write(file_desc, buff, strlen(buff) + 1) == -1) {
        perror("Writing pid to jobExecutorServer.txt failed");
        exit(EXIT_FAILURE);
    }

    // Close the file
    if (close(file_desc) == -1) {
        perror("Closing jobExecutorServer.txt failed");
        exit(EXIT_FAILURE);
    }

    ////////////////////////////////////////////////////////////////////////
    // Signal handler for communication between jobCommander and jobExecutorServer

    // Initiallize the signal handler for SIGUSR1 (signal from jobCommander)
    static struct sigaction action_from_jc;

    // Set the signal handling function to jc_to_jes_sig_handler for when jobCommander starts communicating
    action_from_jc.sa_sigaction = jc_to_jes_sig_handler; 
    action_from_jc.sa_flags = SA_SIGINFO;

    // Block all signals while the signal handler is running
    sigfillset(&(action_from_jc.sa_mask));

    // Set up the signal handler for SIGUSR1
    sigaction(SIGUSR1, &action_from_jc, NULL);


    ////////////////////////////////////////////////////////////////////////
    // Signal handler for the child process that terminates

    // Initiallize the signal handler for SIGCHLD (child process terminating)
    static struct sigaction action_sigchild;
    
    // Set the signal handling function to sigchild_handler for when when SIGCHLD is received
    action_sigchild.sa_handler = sigchild_handler;

    // Block all signals while the signal handler is running
    sigfillset(&(action_sigchild.sa_mask));

    // Set up the signal handler for SIGCHLD
    sigaction(SIGCHLD, &action_sigchild, NULL);


    // Infinite loop to keep the server running
    while (1) {
        sleep(1);
    }

}
