/**
 * \author Lander Van Loock
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <pthread.h>
#include <sys/types.h>
#include <string.h>
#include <unistd.h>

#include "connmgr.h"
#include "datamgr.h"
#include "sensor_db.h"
#include "sbuffer.h"
#include "config.h"

// Define some global variables
int fd[2];
int seq_num = 0;
pid_t pid;

// Define logger functions
int write_to_log_process(char *msg);
int create_log_process();

// Implement logger functions
int write_to_log_process(char *msg) {
    FILE* file = fopen("gateway.log", "a");
    time_t curtime;
    time(&curtime);
    char * timestring = ctime(&curtime);
    timestring[strlen(timestring)-1] = '\0';
    int result = fprintf(file, "%d - %s - %s\n", seq_num, timestring, msg);
    ++seq_num;
    fclose(file);

    return result;
}

int create_log_process() {
    if (pipe(fd) == -1){
        write_to_log_process("Error while creating process communication pipe.");
        return -1;
    }
    pid = fork();
    if (pid < 0) { // Child process creation error
        write_to_log_process("Error while creating child process.");
        return -1;
    }
    if (pid > 0) { // Parent process code
        close(fd[READ_END]);
        return 0;
    }
    else { // Child process code

        close(fd[WRITE_END]);
        char read_msg[SIZE] = "";
        long result = read(fd[READ_END], &read_msg, SIZE);

        while(result >= 0) {
            if(result>0)
                write_to_log_process(read_msg);
            result = read(fd[READ_END], &read_msg, SIZE);
        }
        close(fd[READ_END]);
        exit(EXIT_SUCCESS);
    }
}

int main(int argc, char *argv[]) {

    // Check arguments for connection manager
    if(argc < 3) {
        printf("Please provide the right arguments: first the port, then the max nb of clients");
        return -1;
    }

    // Create a clean logger file at the start of the process
    FILE *f = fopen("gateway.log", "w");
    fclose(f);

    // Create log process
    create_log_process();

    // Thread initialization
    pthread_t tid[3];
    pthread_attr_t attr;
    pthread_attr_init(&attr);

    // Buffer initialization
    sbuffer_t * buffer = NULL;
    sbuffer_init(&buffer);

    // Connection manager parameter initialization
    conn_param * params = malloc(sizeof(conn_param));
    params->port = atoi(argv[1]);
    params->max_conn = atoi(argv[2]);
    params->fd_write = fd[WRITE_END];
    params->buffer = buffer;

    // Other threads parameter initialization
    thread_param * th_params = malloc(sizeof(thread_param));
    th_params->buffer = buffer;
    th_params->fd_write = fd[WRITE_END];

    // Create necessary threads
    pthread_create(&tid[0], &attr, connectionManager, params);
    pthread_create(&tid[1], &attr, dataManager, th_params);
    pthread_create(&tid[2], &attr, storageManager, th_params);

    // Wait for end of threads
    pthread_join(tid[0], NULL);
    pthread_join(tid[1], NULL);
    pthread_join(tid[2], NULL);

    // Free all the memory
    sbuffer_free(&buffer);
    free(params);
    free(th_params);

    close(fd[WRITE_END]);

    return 0;
}