#include <stdio.h>
#include <unistd.h>
#include "sensor_db.h"
#include "logger.h"

// Define pipe constants
#define SIZE 1
#define READ_END 0
#define WRITE_END 1

// Define the non-printable ASCII characters for insert and end message
#define END_MSG 27
#define INSERT_MSG 17

int fd[2];

int create_child();

int create_child() {

    if (pipe(fd) == -1){
        printf("Pipe creation error\n");
        return 1;
    }

    pid_t pid;
    pid = fork();

    if (pid < 0) { // Child process creation error
        printf("Child process creation error\n");
        return 1;
    }

    if (pid > 0) { // Parent process code
        close(fd[READ_END]);
    }

    else { // Child process code
        char read_msg;
        close(fd[WRITE_END]);
        create_log_process();
        write_to_log_process("Data file opened.");
        read(fd[READ_END], &read_msg, SIZE);
        while(read_msg != END_MSG) {
            if(read_msg == INSERT_MSG)
                write_to_log_process("Data inserted.");
            read(fd[READ_END], &read_msg, SIZE);
        }
        write_to_log_process("Data file closed.");
        end_log_process();
        close(fd[READ_END]);
        exit(EXIT_SUCCESS);
    }

    return 0;

}

FILE * open_db(char * filename, bool append) {

    create_child();

    if(append)
        return fopen(filename, "a");
    else
        return fopen(filename, "w");

}

int insert_sensor(FILE * f, sensor_id_t id, sensor_value_t value, sensor_ts_t ts) {

    char write_msg = INSERT_MSG;
    write(fd[WRITE_END], &write_msg, SIZE);

    return fprintf(f, "%d, %f, %ld\n", id, value, ts);

}

int close_db(FILE * f) {

    char write_msg = END_MSG;
    write(fd[WRITE_END], &write_msg, SIZE);
    close(fd[WRITE_END]);

    return fclose(f);

}