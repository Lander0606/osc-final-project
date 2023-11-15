#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include "sensor_db.h"
#include "logger.h"

#define SIZE 20
#define READ_END 0
#define WRITE_END 1

int create_child();

pid_t pid;
int fd[2];


int create_child() {

    if (pipe(fd) == -1){
        printf("Pipe error\n");
        return 1;
    }

    pid = fork();

    if (pid < 0) {
        return 1;
    }
    if (pid > 0) {
        close(fd[READ_END]);

        // write "wait" to process because otherwise read() in child reads incorrect data (20 bytes from fd)
        char write_msg[SIZE] = "wait";
        write(fd[WRITE_END], write_msg, SIZE);
    }
    else {
        char rmsg[SIZE] = "wait";
        printf("First rmsg: %s\n", rmsg);
        close(fd[WRITE_END]);
        create_log_process();
        write_to_log_process("Data file opened.");
        int result = read(fd[READ_END], rmsg, SIZE);
        printf("Result: %d\n", result);
        printf("Result string: %s\n", rmsg);
        while(strcmp(rmsg, "end") != 0) {
            if(strcmp(rmsg, "insert") == 0)
                write_to_log_process("Data inserted.");
            result = read(fd[READ_END], rmsg, SIZE);
            if(result <= 0) {
                strcpy(rmsg, "wait");
            } else {
                printf("Result in loop: %d\n", result);
            }
        }
        write_to_log_process("Data file closed.");
        end_log_process();
        close(fd[READ_END]);
        exit(EXIT_SUCCESS);
    }
    return 0;
}

FILE * open_db(char * filename, bool append) {
    FILE *fp;

    create_child();

    if(append)
        fp = fopen(filename, "a");
    else
        fp = fopen(filename, "w");

    return fp;
}

int insert_sensor(FILE * f, sensor_id_t id, sensor_value_t value, sensor_ts_t ts) {
    char write_msg[SIZE] = "insert";
    printf("Write something to child . . .\n");
    write(fd[WRITE_END], write_msg, SIZE);
    return fprintf(f, "%d, %f, %ld\n", id, value, ts);
}

int close_db(FILE * f) {
    char write_msg[SIZE] = "end";
    write(fd[WRITE_END], write_msg, SIZE);
    close(fd[WRITE_END]);
    return fclose(f);
}