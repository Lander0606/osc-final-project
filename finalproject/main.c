#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <unistd.h>

// Define message pipe constants
#define SIZE 1
#define WRITE_END 1
#define READ_END 0

// Define the non-printable ASCII characters for insert and end message
#define INSERT_MSG 17
#define OPEN_MSG 18
#define CLOSE_MSG 19
#define OPEN_ERR_MSG 20
#define INSERT_ERR_MSG 21
#define CLOSE_ERR_MSG 22
#define END_MSG 23

// Define some global variables
int fd[2];
int seq_num = 0;
pid_t pid;

// Define logger functions
int write_to_log_process(char *msg);

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

int main(int argc, char *argv[]) {

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
    }

    else { // Child process code

        char read_msg;
        close(fd[WRITE_END]);
        read(fd[READ_END], &read_msg, SIZE);

        while(read_msg != END_MSG) {

            if(read_msg == INSERT_MSG)
                write_to_log_process("Data inserted.");
            else if(read_msg == INSERT_ERR_MSG)
                write_to_log_process("Error while writing to the file.");
            else if(read_msg == OPEN_MSG)
                write_to_log_process("Data file opened.");
            else if(read_msg == OPEN_ERR_MSG)
                write_to_log_process("Error while opening file: database file already opened");
            else if(read_msg == CLOSE_ERR_MSG)
                write_to_log_process("Error while closing file.");
            else if(read_msg == CLOSE_MSG) {
                write_to_log_process("Data file closed.");
                break;
            }

            read(fd[READ_END], &read_msg, SIZE);
        }

        close(fd[READ_END]);
        exit(EXIT_SUCCESS);

    }

    return 0;

}